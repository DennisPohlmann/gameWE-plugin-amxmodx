/**
 * gameWE - AMX Mod X plugin, shared core
 *
 * Server commands for the gameWE collector (chat, HUD, menus, MOTD, team
 * swap), the gameWE menu and the hidden stats chat commands. Every mod
 * plugin (gamewe_cstrike.sma, gamewe_dod.sma, ...) includes this file and
 * defines the mod specific functions:
 *
 *   gamewe_mod_init()                       end of plugin_init
 *   gamewe_mod_chat(player, message[], colored, green)
 *                                           private chat message
 *   gamewe_mod_swap(player)                 move a player to the other team
 *
 * Define GAMEWE_FULL_MENU before including this file for the long menu
 * (rank, clans, weapons, ...); otherwise the short menu is used.
 *
 * Define GAMEWE_DAMAGE_DISPLAY for mods with damage data (csx, dodx, tfcx,
 * see gamewe_xstats.inc): adds the cvars gamewe_damage_display(_allow) and
 * the per-player "Damage display" setting.
 *
 * Every mod keeps the players' own settings in the nVault "gamewe_settings":
 * console and chat events (gamewe_display, gamewe_chat), auto commands
 * (gamewe_auto) and the language (gamewe_lang, include gamewe_lang.inc).
 *
 * Fork of the gameME Plugin 2.4 for AMX Mod X
 * (https://github.com/gamemedev/plugin-amxmodx),
 * Copyright (C) 2007-2017 TTS Oetzel & Goerz GmbH, licensed under the
 * GNU General Public License version 2 or (at your option) any later version.
 * Copyright (C) 2026 gameWE contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#if defined _gamewe_core_included
	#endinput
#endif
#define _gamewe_core_included

// the server commands keep up to three 1024 cell buffers on the stack
#pragma dynamic 8192

#include <amxmodx>
#include <amxmisc>
#include <celltrie>
#include <nvault>

// written by "make build" from the git tag or the VERSION file
#tryinclude <gamewe_version>
#if !defined GAMEWE_PLUGIN_VERSION
	#define GAMEWE_PLUGIN_VERSION "1.0.0"
#endif

#define GAMEWE_TAG          "gameWE"
#define GAMEWE_CHAT_TAG     "[gameWE]" // start of the collector's chat lines
#define GAMEWE_MAX_PREFIX   64
#define GAMEWE_MAX_ARGS     1024    // raw argument string of a server command
#define GAMEWE_MAX_CHAT     192     // SayText / TextMsg limit of the engine
#define GAMEWE_MAX_MENU     512     // show_menu limit of the engine
#define GAMEWE_MAX_URL      512
#define GAMEWE_MAX_RECIPIENTS MAX_PLAYERS

// chat commands answered by the collector, also blocked with "/" and "!" in front
stock const gamewe_default_blocked_commands[][] = {
	"rank", "skill", "points", "place", "session", "sdata", "session_data", "kpd", "kdratio", "kdeath",
	"next", "load", "status", "servers", "top20", "top10", "top5", "maps", "map_stats", "clans",
	"cheaters", "statsme", "weapons", "weapon", "action", "actions", "accuracy",
	"targets", "target", "kills", "kill", "player_kills", "cmd", "cmds", "commands", "help",
	"gstats", "global_stats", "gamewe", "gamewe_menu", "gamewe_display 0", "gamewe_display 1",
	"gamewe_atb 0", "gamewe_atb 1", "gamewe_hideranking", "gamewe_reset", "gamewe_reset 0", "gamewe_reset 1",
	"gamewe_chat 0", "gamewe_chat 1",
	"hideranking", "hideranking 0", "hideranking 1", "hideranking on", "hideranking off",
	"hideranking hide", "hideranking show",
	"gamewe_hideranking 0", "gamewe_hideranking 1", "gamewe_hideranking on", "gamewe_hideranking off",
	"gamewe_hideranking hide", "gamewe_hideranking show",
	"gameme_hideranking 0", "gameme_hideranking 1", "gameme_hideranking on", "gameme_hideranking off",
	"gameme_hideranking hide", "gameme_hideranking show",
	"gameme", "gameme_menu", "gameme_display 0", "gameme_display 1",
	"gameme_atb 0", "gameme_atb 1", "gameme_hideranking", "gameme_reset", "gameme_reset 0", "gameme_reset 1",
	"gameme_chat 0", "gameme_chat 1"
}

// chat commands that open the gameWE menu
stock const gamewe_menu_commands[][] = {
	"gamewe", "/gamewe", "!gamewe", "gamewe_menu", "/gamewe_menu", "!gamewe_menu",
	"gameme", "/gameme", "!gameme", "gameme_menu", "/gameme_menu", "!gameme_menu"
}

// chat commands that open the gameWE settings menu; hidden and not logged,
// so the collector does not answer them as well
stock const gamewe_settings_commands[][] = {
	"settings", "/settings", "!settings", "gamewe_settings", "/gamewe_settings", "!gamewe_settings"
}

new g_gamewe_message_prefix
new g_gamewe_chat_cooldown
new Trie:g_gamewe_blocked_commands
new g_gamewe_menu_main
new g_gamewe_menu_settings
new Float:g_gamewe_last_trigger[MAX_PLAYERS + 1]

// ranking state from the collector (gamewe_hidestate): 0 = shown, 1 = admin,
// 2 = banned, 3 = hidden by the player, -1 = not known yet
#define GAMEWE_HIDERANKING_UNKNOWN -1
new g_gamewe_hideranking[MAX_PLAYERS + 1] = { GAMEWE_HIDERANKING_UNKNOWN, ... }
new bool:g_gamewe_hideranking_wait[MAX_PLAYERS + 1]
new Float:g_gamewe_hideranking_asked[MAX_PLAYERS + 1] // last request, at most one per 5 seconds

// per-player settings: -1 = server default (cvar), otherwise the player's own choice
#define GAMEWE_SETTING_DEFAULT   -1
#define GAMEWE_SETTINGS_VAULT    "gamewe_settings"
#define GAMEWE_SETTINGS_MAX_AGE  (180 * 86400)  // settings of players not seen for 180 days are removed

new g_gamewe_settings_vault = INVALID_HANDLE

// gamewe_display / gamewe_chat: console and chat events of the collector, 1 = on (default)
new g_gamewe_console_events[MAX_PLAYERS + 1] = { 1, ... }
new g_gamewe_global_chat[MAX_PLAYERS + 1] = { 1, ... }

// gamewe_auto: the command sent for the player at round start, round end and death
#define GAMEWE_AUTO_START   0
#define GAMEWE_AUTO_END     1
#define GAMEWE_AUTO_KILL    2
#define GAMEWE_AUTO_EVENTS  3
#define GAMEWE_AUTO_LENGTH  16
// slot of a player's command: GAMEWE_AUTO_SLOT(id, event)
#define GAMEWE_AUTO_SLOT(%1,%2) ((%1) * GAMEWE_AUTO_EVENTS + (%2))
new g_gamewe_auto[(MAX_PLAYERS + 1) * GAMEWE_AUTO_EVENTS][GAMEWE_AUTO_LENGTH]
new Float:g_gamewe_last_auto[MAX_PLAYERS + 1][GAMEWE_AUTO_EVENTS]

// the stats commands of the collector that may be auto commands, never free text
stock const gamewe_auto_commands[][] = {
	"rank", "skill", "points", "kpd", "kdratio", "session", "sdata", "next",
	"top5", "top10", "top20", "statsme"
}
stock const gamewe_auto_types[GAMEWE_AUTO_EVENTS][] = { "start", "end", "kill" }

#if defined GAMEWE_DAMAGE_DISPLAY
new g_gamewe_damage_display
new g_gamewe_damage_display_allow
new g_gamewe_player_damage_display[MAX_PLAYERS + 1] = { GAMEWE_SETTING_DEFAULT, ... }
#endif

#include <gamewe_lang>

stock const GAMEWE_MENU_KEYS = MENU_KEY_0|MENU_KEY_1|MENU_KEY_2|MENU_KEY_3|MENU_KEY_4|MENU_KEY_5|MENU_KEY_6|MENU_KEY_7|MENU_KEY_8|MENU_KEY_9
#define GAMEWE_MENU_TITLE "gameWE Display Menu"


/*
 * Setup, called by the mod plugin from plugin_init.
 */
stock gamewe_core_init(const mod_name[])
{
	new plugin_version[48]
	formatex(plugin_version, charsmax(plugin_version), "%s (HL1 %s)", GAMEWE_PLUGIN_VERSION, mod_name)
	register_plugin(GAMEWE_TAG, GAMEWE_PLUGIN_VERSION, "gameWE contributors, based on gameME by TTS Oetzel & Goerz GmbH")
	create_cvar("gamewe_plugin_version", plugin_version, FCVAR_SPONLY|FCVAR_SERVER, "gameWE plugin version (read only)")

	g_gamewe_message_prefix = create_cvar("gamewe_message_prefix", "", FCVAR_NONE,
		"Prefix of every gameWE chat message, also of the collector's answers (up to 63 characters, color tags like {green} or {default} allowed on Counter-Strike)")
	g_gamewe_chat_cooldown = create_cvar("gamewe_chat_cooldown", "1.0", FCVAR_NONE,
		"Seconds a player has to wait between two logged gameWE chat commands (0-30, 0 = off)", true, 0.0, true, 30.0)
#if defined GAMEWE_DAMAGE_DISPLAY
	g_gamewe_damage_display = create_cvar("gamewe_damage_display", "0", FCVAR_NONE,
		"Default for players: damage summary on death (0 = off, 1 = menu, 2 = chat); players can change it in the gameWE settings menu", true, 0.0, true, 2.0)
	g_gamewe_damage_display_allow = create_cvar("gamewe_damage_display_allow", "1", FCVAR_NONE,
		"Allow the damage display at all (0 = off for everyone, players cannot enable it)", true, 0.0, true, 1.0)
#endif

	g_gamewe_settings_vault = nvault_open(GAMEWE_SETTINGS_VAULT)
	if (g_gamewe_settings_vault == INVALID_HANDLE) {
		log_amx("gameWE: cannot open the nVault %s, player settings are not saved", GAMEWE_SETTINGS_VAULT)
	} else {
		nvault_prune(g_gamewe_settings_vault, 0, get_systime() - GAMEWE_SETTINGS_MAX_AGE)
	}
	AutoExecConfig(true, "gamewe")

	g_gamewe_blocked_commands = TrieCreate()
	new prefixed[64]
	for (new i = 0; i < sizeof(gamewe_default_blocked_commands); i++) {
		TrieSetCell(g_gamewe_blocked_commands, gamewe_default_blocked_commands[i], 1)
		formatex(prefixed, charsmax(prefixed), "/%s", gamewe_default_blocked_commands[i])
		TrieSetCell(g_gamewe_blocked_commands, prefixed, 1)
		formatex(prefixed, charsmax(prefixed), "!%s", gamewe_default_blocked_commands[i])
		TrieSetCell(g_gamewe_blocked_commands, prefixed, 1)
	}

	// server console / RCON only; every command in three spellings
	gamewe_register_srvcmd("psay",   "gamewe_cmd_psay",   "<userid[,userid...]> [colored] <message> - private chat message")
	gamewe_register_srvcmd("psay2",  "gamewe_cmd_psay2",  "<userid[,userid...]> [colored] <message> - green private chat message")
	gamewe_register_srvcmd("say",    "gamewe_cmd_say",    "<message> - chat message to all players")
	gamewe_register_srvcmd("csay",   "gamewe_cmd_csay",   "<message> - center message to all players")
	gamewe_register_srvcmd("hint",   "gamewe_cmd_hint",   "<userid[,userid...]> <message> - HUD message")
	gamewe_register_srvcmd("msay",   "gamewe_cmd_msay",   "<time> <userid> [handler] <message> - menu")
	gamewe_register_srvcmd("browse", "gamewe_cmd_browse", "<userid> <url> - open a website in the MOTD window")
	gamewe_register_srvcmd("swap",   "gamewe_cmd_swap",   "<userid> - move a player to the opposite team")
	gamewe_register_srvcmd("hidestate", "gamewe_cmd_hidestate", "<userid> <hideranking> - ranking state of a player for the settings menu")
	gamewe_register_srvcmd("console_event", "gamewe_cmd_console_event", "<userid[,userid...]> <message> - console line for players with gamewe_display 1")
	gamewe_register_srvcmd("chat_event", "gamewe_cmd_chat_event", "<userid[,userid...]> <message> - chat line for players with gamewe_chat 1")
	register_srvcmd("gamewe_block_commands_values", "gamewe_cmd_block_values", -1, "<command ...|clear> - more chat commands that are silent with /")
	register_srvcmd("gameme_block_commands_values", "gamewe_cmd_block_values")
	register_srvcmd("gamewe_message_prefix_clear", "gamewe_cmd_prefix_clear", -1, "- empty gamewe_message_prefix")

	register_clcmd("say",      "gamewe_client_say")
	register_clcmd("say_team", "gamewe_client_say")
	register_clcmd("amx_gamewe_settings", "gamewe_client_settings", -1, "- open the gameWE settings menu")
	register_menucmd(register_menuid(GAMEWE_MENU_TITLE), GAMEWE_MENU_KEYS, "gamewe_display_menu_handle")

	// gamewe_auto: round start and end as the games log them, deaths of everybody
	register_logevent("gamewe_auto_round_start", 2, "1=Round_Start")
	register_logevent("gamewe_auto_round_end", 2, "1=Round_End")
	register_event("DeathMsg", "gamewe_auto_death", "a")

	gamewe_build_menus()
}


stock gamewe_register_srvcmd(const name[], const handler[], const info[])
{
	new command[32]
	formatex(command, charsmax(command), "gamewe_amx_%s", name)
	register_srvcmd(command, handler, -1, info)
	formatex(command, charsmax(command), "gameme_amx_%s", name)
	register_srvcmd(command, handler)
	formatex(command, charsmax(command), "gamewe_%s", name)
	register_srvcmd(command, handler)
}


/*
 * Logging
 */

// "Name<userid><authid><team>" <verb> "<event>", the HL log format
stock gamewe_log_player_event(client, const verb[], const player_event[])
{
	if (!is_user_connected(client)) {
		return
	}
	new name[MAX_NAME_LENGTH], authid[MAX_AUTHID_LENGTH], team[16]
	get_user_name(client, name, charsmax(name))
	get_user_authid(client, authid, charsmax(authid))
	get_user_team(client, team, charsmax(team))
	log_message("^"%s<%d><%s><%s>^" %s ^"%s^"", name, get_user_userid(client), authid, team, verb, player_event)
}

// "Name<userid><authid><team>" requested "<query>" (value "0"): asks the collector
stock gamewe_log_player_request(client, const query[])
{
	if (!is_user_connected(client)) {
		return
	}
	new name[MAX_NAME_LENGTH], authid[MAX_AUTHID_LENGTH], team[16]
	get_user_name(client, name, charsmax(name))
	get_user_authid(client, authid, charsmax(authid))
	get_user_team(client, team, charsmax(team))
	log_message("^"%s<%d><%s><%s>^" requested ^"%s^" (value ^"0^")", name, get_user_userid(client), authid, team, query)
}


// "Name<userid><authid><team>" setup "<name>" (value "<value>"): a setting
// for the collector; name and value are fixed strings of the plugin
stock gamewe_log_player_setup(client, const setting[], const value[])
{
	if (!is_user_connected(client)) {
		return
	}
	new name[MAX_NAME_LENGTH], authid[MAX_AUTHID_LENGTH], team[16]
	get_user_name(client, name, charsmax(name))
	get_user_authid(client, authid, charsmax(authid))
	get_user_team(client, team, charsmax(team))
	log_message("^"%s<%d><%s><%s>^" setup ^"%s^" (value ^"%s^")", name, get_user_userid(client), authid, team, setting, value)
}


/*
 * Parsing of the collector's commands
 *
 * The HL tokenizer splits at ( ) { } ' : and drops quotes, so the arguments
 * are parsed from the raw argument string instead of read_argv().
 */

// reads the next token from text at pos; quoted tokens may contain spaces
stock gamewe_next_token(const text[], &pos, token[], len)
{
	token[0] = EOS
	while (text[pos] != EOS && text[pos] <= ' ') {
		pos++
	}
	if (text[pos] == EOS) {
		return 0
	}
	new n = 0
	if (text[pos] == '"') {
		pos++
		while (text[pos] != EOS && text[pos] != '"') {
			if (n < len) {
				token[n++] = text[pos]
			}
			pos++
		}
		if (text[pos] == '"') {
			pos++
		}
	} else {
		while (text[pos] > ' ') {
			if (n < len) {
				token[n++] = text[pos]
			}
			pos++
		}
	}
	token[n] = EOS
	return 1
}

// the rest of text from pos: one pair of surrounding quotes removed, control characters dropped
stock gamewe_rest_of(const text[], pos, out[], len)
{
	while (text[pos] != EOS && text[pos] <= ' ') {
		pos++
	}
	new n = 0
	for (new i = pos; text[i] != EOS && n < len; i++) {
		if (text[i] >= ' ') {
			out[n++] = text[i]
		}
	}
	out[n] = EOS
	trim(out)
	n = strlen(out)
	if (n >= 2 && out[0] == '"' && out[n - 1] == '"') {
		out[n - 1] = EOS
		copy(out, len, out[1])
	} else if (n >= 1 && out[0] == '"') {
		copy(out, len, out[1])
	}
}

// "0" or "1", used for the optional colored / handler argument
stock bool:gamewe_is_flag(const token[])
{
	return (equal(token, "0") || equal(token, "1"))
}

// player index of a userid ("12" or "#12"), 0 if not in game; bots and HLTV are skipped
stock gamewe_player_of_userid(const token[])
{
	new start = (token[0] == '#') ? 1 : 0
	if (token[start] == EOS || !is_str_num(token[start]) || strlen(token[start]) > 9) {
		return 0
	}
	new userid = str_to_num(token[start])
	if (userid <= 0) {
		return 0
	}
	for (new player = 1; player <= MaxClients; player++) {
		if (is_user_connected(player) && !is_user_bot(player) && !is_user_hltv(player) && get_user_userid(player) == userid) {
			return player
		}
	}
	return 0
}

// list of userids "1,2,#3" into player indexes, duplicates removed; returns the count
stock gamewe_parse_recipients(const token[], players[GAMEWE_MAX_RECIPIENTS])
{
	new count = 0
	new part[16]
	new pos = 0
	new length = strlen(token)
	while (pos < length && count < GAMEWE_MAX_RECIPIENTS) {
		new n = 0
		while (pos < length && token[pos] != ',') {
			if (n < charsmax(part)) {
				part[n++] = token[pos]
			}
			pos++
		}
		part[n] = EOS
		pos++

		new player = gamewe_player_of_userid(part)
		if (player > 0) {
			new duplicate = 0
			for (new i = 0; i < count; i++) {
				if (players[i] == player) {
					duplicate = 1
					break
				}
			}
			if (!duplicate) {
				players[count++] = player
			}
		}
	}
	return count
}

// "<userid[,userid...]> [0|1] <message>"
stock gamewe_read_recipients_message(players[GAMEWE_MAX_RECIPIENTS], &count, &flag, message[], len, bool:with_flag)
{
	new args[GAMEWE_MAX_ARGS], token[192]
	read_args(args, charsmax(args))

	new pos = 0
	gamewe_next_token(args, pos, token, charsmax(token))
	count = gamewe_parse_recipients(token, players)

	flag = 0
	if (with_flag) {
		new flag_pos = pos
		new flag_token[8]
		gamewe_next_token(args, flag_pos, flag_token, charsmax(flag_token))
		if (gamewe_is_flag(flag_token)) {
			flag = str_to_num(flag_token)
			pos = flag_pos
		}
	}
	gamewe_rest_of(args, pos, message, len)
}

// Color tags of the message prefix. Counter-Strike has the default color
// (^1), the team color (^3) and green (^4); other colors become the team
// color or green, colors the game cannot show are removed. Without colors
// (other mods) every tag is removed.
stock const gamewe_color_tag_names[][] = {
	"{default}", "{teamcolor}", "{team}", "{green}", "{lightgreen}", "{olive}",
	"{red}", "{lightred}", "{darkred}", "{blue}", "{lightblue}", "{grey}",
	"{gray}", "{yellow}", "{orange}", "{purple}", "{lime}"
}
stock const gamewe_color_tag_codes[][] = {
	"^1", "^3", "^3", "^4", "^4", "^4",
	"^3", "^3", "^3", "^3", "^3", "^3",
	"^3", "", "", "", "^4"
}

stock gamewe_color_tags(text[], len, bool:colors)
{
	if (contain(text, "{") == -1) {
		return
	}
	for (new i = 0; i < sizeof(gamewe_color_tag_names); i++) {
		replace_string(text, len, gamewe_color_tag_names[i], colors ? gamewe_color_tag_codes[i] : "", false)
	}
}

// the message prefix; colors: color tags become CS color codes, otherwise
// all color tags are removed
stock gamewe_get_prefix(prefix[], len, bool:colors = false)
{
	get_pcvar_string(g_gamewe_message_prefix, prefix, len)
	trim(prefix)
	gamewe_color_tags(prefix, len, colors)
	if (colors) {
		replace_string(prefix, len, "x01", "^1")
		replace_string(prefix, len, "x03", "^3")
		replace_string(prefix, len, "x04", "^4")
	}
	gamewe_remove_color_tags(prefix, len)
}

// The collector starts its chat lines with [gameWE]; with a message prefix
// set, the tag is removed and the mod's chat puts the prefix in front.
stock gamewe_strip_chat_tag(message[], len)
{
	new prefix[GAMEWE_MAX_PREFIX]
	gamewe_get_prefix(prefix, charsmax(prefix))
	new tag_length = strlen(GAMEWE_CHAT_TAG)
	if (prefix[0] == EOS || !equal(message, GAMEWE_CHAT_TAG, tag_length)) {
		return
	}
	new start = tag_length
	while (message[start] == ' ') {
		start++
	}
	new rest[GAMEWE_MAX_CHAT]
	copy(rest, charsmax(rest), message[start])
	copy(message, len, rest)
}

// default chat for mods without colors
stock gamewe_plain_chat(player, const message[])
{
	new prefix[GAMEWE_MAX_PREFIX]
	gamewe_get_prefix(prefix, charsmax(prefix))
	if (prefix[0] != EOS) {
		client_print(player, print_chat, "%s %s", prefix, message)
	} else {
		client_print(player, print_chat, "%s", message)
	}
}

// removes the gameME color tags x01 ... x08 from a message
stock gamewe_remove_color_tags(message[], len)
{
	replace_string(message, len, "x01", "")
	replace_string(message, len, "x02", "")
	replace_string(message, len, "x03", "")
	replace_string(message, len, "x04", "")
	replace_string(message, len, "x05", "")
	replace_string(message, len, "x06", "")
	replace_string(message, len, "x07", "")
	replace_string(message, len, "x08", "")
}


/*
 * Server commands for the collector
 */

public gamewe_cmd_psay(id, level, cid)
{
	gamewe_psay(false)
	return PLUGIN_HANDLED
}

public gamewe_cmd_psay2(id, level, cid)
{
	gamewe_psay(true)
	return PLUGIN_HANDLED
}

stock gamewe_psay(bool:green)
{
	new players[GAMEWE_MAX_RECIPIENTS], count, colored
	new message[GAMEWE_MAX_CHAT]
	gamewe_read_recipients_message(players, count, colored, message, charsmax(message), true)
	if (message[0] == EOS) {
		return
	}
	gamewe_strip_chat_tag(message, charsmax(message))
	for (new i = 0; i < count; i++) {
		gamewe_mod_chat(players[i], message, colored, green)
	}
}

public gamewe_cmd_say(id, level, cid)
{
	new args[GAMEWE_MAX_ARGS], message[GAMEWE_MAX_CHAT]
	read_args(args, charsmax(args))
	gamewe_rest_of(args, 0, message, charsmax(message))
	gamewe_remove_color_tags(message, charsmax(message))
	if (message[0] != EOS) {
		gamewe_plain_chat(0, message)
	}
	return PLUGIN_HANDLED
}

public gamewe_cmd_csay(id, level, cid)
{
	new args[GAMEWE_MAX_ARGS], message[GAMEWE_MAX_CHAT]
	read_args(args, charsmax(args))
	gamewe_rest_of(args, 0, message, charsmax(message))
	gamewe_remove_color_tags(message, charsmax(message))
	if (message[0] != EOS) {
		set_hudmessage(255, 255, 255, -1.0, 0.3, 0, 6.0, 6.0, 0.5, 0.15, -1)
		show_hudmessage(0, "%s", message)
	}
	return PLUGIN_HANDLED
}

public gamewe_cmd_hint(id, level, cid)
{
	new players[GAMEWE_MAX_RECIPIENTS], count, flag
	new message[GAMEWE_MAX_CHAT]
	gamewe_read_recipients_message(players, count, flag, message, charsmax(message), false)
	gamewe_remove_color_tags(message, charsmax(message))
	if (message[0] == EOS) {
		return PLUGIN_HANDLED
	}
	for (new i = 0; i < count; i++) {
		set_hudmessage(255, 128, 0, -1.0, 0.80, 0, 6.0, 6.0, 0.5, 0.15, -1)
		show_hudmessage(players[i], "%s", message)
	}
	return PLUGIN_HANDLED
}

// "<time> <userid> [handler] <text>", lines separated by \n or ^n
public gamewe_cmd_msay(id, level, cid)
{
	new args[GAMEWE_MAX_ARGS], token[32]
	read_args(args, charsmax(args))

	new pos = 0
	gamewe_next_token(args, pos, token, charsmax(token))
	new display_time = is_str_num(token) ? str_to_num(token) : 0
	if (display_time <= 0 || display_time > 120) {
		display_time = 15
	}

	gamewe_next_token(args, pos, token, charsmax(token))
	new player = gamewe_player_of_userid(token)

	new handler = 0
	new flag_pos = pos
	gamewe_next_token(args, flag_pos, token, charsmax(token))
	if (gamewe_is_flag(token)) {
		handler = str_to_num(token)
		pos = flag_pos
	}

	new text[GAMEWE_MAX_MENU]
	gamewe_rest_of(args, pos, text, charsmax(text))
	replace_string(text, charsmax(text), "\n", "^n")
	replace_string(text, charsmax(text), "^^n", "^n")
	gamewe_remove_color_tags(text, charsmax(text))

	if (player > 0 && text[0] != EOS) {
		if (handler == 1) {
			show_menu(player, GAMEWE_MENU_KEYS, text, display_time, GAMEWE_MENU_TITLE)
		} else {
			show_menu(player, GAMEWE_MENU_KEYS, text, display_time)
		}
	}
	return PLUGIN_HANDLED
}

public gamewe_display_menu_handle(id, key)
{
	if (is_user_connected(id)) {
		if (key >= 0 && key < 9) {
			new slot[4]
			num_to_str(key + 1, slot, charsmax(slot))
			gamewe_log_player_event(id, "selected", slot)
		} else {
			gamewe_log_player_event(id, "selected", "cancel")
		}
	}
	return PLUGIN_HANDLED
}

// only http(s) URLs: show_motd() shows a file of the server for anything else
stock bool:gamewe_is_valid_url(const url[])
{
	new length = strlen(url)
	if (length < 8 || length >= GAMEWE_MAX_URL) {
		return false
	}
	if (!(equali(url, "http://", 7) || equali(url, "https://", 8))) {
		return false
	}
	for (new i = 0; i < length; i++) {
		if (url[i] <= ' ' || url[i] == '"' || url[i] == '<' || url[i] == '>' || url[i] == '^^' || url[i] > 126) {
			return false
		}
	}
	return true
}

public gamewe_cmd_browse(id, level, cid)
{
	new args[GAMEWE_MAX_ARGS], token[32], url[GAMEWE_MAX_URL + 1]
	read_args(args, charsmax(args))

	new pos = 0
	gamewe_next_token(args, pos, token, charsmax(token))
	new player = gamewe_player_of_userid(token)
	gamewe_rest_of(args, pos, url, charsmax(url))

	if (player > 0) {
		if (gamewe_is_valid_url(url)) {
			show_motd(player, url, "gameWE Stats")
		} else {
			log_amx("gameWE: browse ignored, only http:// and https:// URLs are allowed")
		}
	}
	return PLUGIN_HANDLED
}

public gamewe_cmd_swap(id, level, cid)
{
	new args[GAMEWE_MAX_ARGS], token[32]
	read_args(args, charsmax(args))
	new pos = 0
	gamewe_next_token(args, pos, token, charsmax(token))
	new player = gamewe_player_of_userid(token)
	if (player > 0) {
		gamewe_mod_swap(player)
	}
	return PLUGIN_HANDLED
}

// "<command> <command> ..." adds chat commands to hide, "clear" removes all
public gamewe_cmd_block_values(id, level, cid)
{
	new args[GAMEWE_MAX_ARGS], token[64]
	read_args(args, charsmax(args))
	remove_quotes(args)
	trim(args)

	if (equal(args, "clear")) {
		TrieClear(g_gamewe_blocked_commands)
		log_message("Server triggered ^"%s^"", "blocked_commands_cleared")
		return PLUGIN_HANDLED
	}

	new pos = 0, added = 0
	while (added < 32 && gamewe_next_token(args, pos, token, charsmax(token))) {
		if (token[0] != EOS) {
			TrieSetCell(g_gamewe_blocked_commands, token, 1)
			added++
		}
	}
	return PLUGIN_HANDLED
}

// "<userid> <hideranking>": the collector tells the player's ranking state,
// after !hideranking / !settings or when the settings menu asked for it
public gamewe_cmd_hidestate(id, level, cid)
{
	new args[GAMEWE_MAX_ARGS], token[16]
	read_args(args, charsmax(args))
	new pos = 0
	gamewe_next_token(args, pos, token, charsmax(token))
	new player = gamewe_player_of_userid(token)
	if (player <= 0) {
		return PLUGIN_HANDLED
	}
	gamewe_next_token(args, pos, token, charsmax(token))
	new hide_state = (is_str_num(token) && strlen(token) == 1) ? str_to_num(token) : GAMEWE_HIDERANKING_UNKNOWN
	if (hide_state > 3) {
		hide_state = GAMEWE_HIDERANKING_UNKNOWN
	}
	g_gamewe_hideranking[player] = hide_state
	if (g_gamewe_hideranking_wait[player]) {
		g_gamewe_hideranking_wait[player] = false
		menu_display(player, g_gamewe_menu_settings, 0)
	}
	return PLUGIN_HANDLED
}

// "<userid[,userid...]> <message>": the collector's points of kills and
// actions, for the listed players whose console events are on
public gamewe_cmd_console_event(id, level, cid)
{
	new players[GAMEWE_MAX_RECIPIENTS], count, flag
	new message[GAMEWE_MAX_CHAT]
	gamewe_read_recipients_message(players, count, flag, message, charsmax(message), false)
	gamewe_remove_color_tags(message, charsmax(message))
	if (message[0] == EOS) {
		return PLUGIN_HANDLED
	}
	for (new i = 0; i < count; i++) {
		if (g_gamewe_console_events[players[i]] == 1) {
			client_print(players[i], print_console, "%s %s", GAMEWE_CHAT_TAG, message)
		}
	}
	return PLUGIN_HANDLED
}

// "<userid[,userid...]> <message>": the collector's announcements of
// actions with points, for the listed players whose global chat is on
public gamewe_cmd_chat_event(id, level, cid)
{
	new players[GAMEWE_MAX_RECIPIENTS], count, flag
	new message[GAMEWE_MAX_CHAT]
	gamewe_read_recipients_message(players, count, flag, message, charsmax(message), false)
	if (message[0] == EOS) {
		return PLUGIN_HANDLED
	}
	gamewe_strip_chat_tag(message, charsmax(message))
	for (new i = 0; i < count; i++) {
		if (g_gamewe_global_chat[players[i]] == 1) {
			gamewe_mod_chat(players[i], message, 0, false)
		}
	}
	return PLUGIN_HANDLED
}

public gamewe_cmd_prefix_clear(id, level, cid)
{
	set_pcvar_string(g_gamewe_message_prefix, "")
	return PLUGIN_HANDLED
}


/*
 * Player chat: hidden stats commands and the gameWE menu
 */

public client_putinserver(id)
{
	g_gamewe_last_trigger[id] = 0.0
	g_gamewe_hideranking[id] = GAMEWE_HIDERANKING_UNKNOWN
	g_gamewe_hideranking_wait[id] = false
	g_gamewe_hideranking_asked[id] = 0.0
	for (new i = 0; i < GAMEWE_AUTO_EVENTS; i++) {
		g_gamewe_last_auto[id][i] = 0.0
	}
	gamewe_load_player_settings(id)
	gamewe_apply_language(id)
}

public gamewe_client_say(client)
{
	if (client <= 0 || client > MaxClients || !is_user_connected(client)) {
		return PLUGIN_CONTINUE
	}

	new said[GAMEWE_MAX_CHAT]
	read_args(said, charsmax(said))
	remove_quotes(said)
	trim(said)
	if (said[0] == EOS) {
		return PLUGIN_CONTINUE
	}

	for (new i = 0; i < sizeof(gamewe_menu_commands); i++) {
		if (equali(said, gamewe_menu_commands[i])) {
			gamewe_display_menu(client, g_gamewe_menu_main)
			break
		}
	}
	for (new i = 0; i < sizeof(gamewe_settings_commands); i++) {
		if (equali(said, gamewe_settings_commands[i])) {
			gamewe_display_settings_menu(client)
			return PLUGIN_HANDLED
		}
	}
	if (gamewe_player_setting(client, said)) {
		// kept by the plugin; "/..." is silent, "!..." stays in chat
		return (said[0] == '/') ? PLUGIN_HANDLED : PLUGIN_CONTINUE
	}

	// "/" commands are silent: hidden from chat and logged by the plugin.
	// "!" commands and plain words stay visible, the game logs them itself.
	if (said[0] != '/' || !(TrieKeyExists(g_gamewe_blocked_commands, said) || TrieKeyExists(g_gamewe_blocked_commands, said[1]))) {
		return PLUGIN_CONTINUE
	}

	// logged for the collector unless the player floods
	new Float:now = get_gametime()
	new Float:cooldown = get_pcvar_float(g_gamewe_chat_cooldown)
	if (cooldown <= 0.0 || g_gamewe_last_trigger[client] <= 0.0 || (now - g_gamewe_last_trigger[client]) >= cooldown || now < g_gamewe_last_trigger[client]) {
		g_gamewe_last_trigger[client] = now
		gamewe_log_player_event(client, "say", said)
	}
	return PLUGIN_HANDLED
}

// console command amx_gamewe_settings, opens the settings menu for the player
public gamewe_client_settings(client)
{
	if (client >= 1 && client <= MaxClients && is_user_connected(client)) {
		gamewe_display_settings_menu(client)
	}
	return PLUGIN_HANDLED
}


/*
 * Per-player settings, saved in the nVault "gamewe_settings" as
 * "<authid>#<setting>". LAN, pending and bot ids are shared by several
 * players, so their choices are not saved and last until the next map.
 */

public client_authorized(id, const authid[])
{
	gamewe_load_player_settings(id)
	// already in game when the Steam id arrives late (else client_putinserver)
	gamewe_apply_language(id)
}

public plugin_end()
{
	if (g_gamewe_settings_vault != INVALID_HANDLE) {
		nvault_close(g_gamewe_settings_vault)
		g_gamewe_settings_vault = INVALID_HANDLE
	}
}

stock bool:gamewe_settings_key(id, const setting[], key[], len)
{
	if (g_gamewe_settings_vault == INVALID_HANDLE || !is_user_connected(id) || is_user_bot(id) || is_user_hltv(id)) {
		return false
	}
	new authid[MAX_AUTHID_LENGTH]
	get_user_authid(id, authid, charsmax(authid))
	if (contain(authid, ":") == -1) {
		return false
	}
	formatex(key, len, "%s#%s", authid, setting)
	return true
}

// the stored value of a setting, false if there is none
stock bool:gamewe_lookup_setting(id, const setting[], value[], len)
{
	new key[MAX_AUTHID_LENGTH + 32], timestamp
	value[0] = EOS
	if (!gamewe_settings_key(id, setting, key, charsmax(key))) {
		return false
	}
	if (!nvault_lookup(g_gamewe_settings_vault, key, value, len, timestamp)) {
		return false
	}
	// keeps the entry from being pruned
	nvault_touch(g_gamewe_settings_vault, key)
	return true
}

stock gamewe_load_player_settings(id)
{
	if (id < 1 || id > MaxClients) {
		return
	}
	g_gamewe_console_events[id] = 1
	g_gamewe_global_chat[id] = 1
	for (new i = 0; i < GAMEWE_AUTO_EVENTS; i++) {
		g_gamewe_auto[GAMEWE_AUTO_SLOT(id, i)][0] = EOS
	}
	g_gamewe_lang[id] = GAMEWE_LANG_UNSET

	new value[64]
	if (gamewe_lookup_setting(id, "console_events", value, charsmax(value)) && equal(value, "0")) {
		g_gamewe_console_events[id] = 0
	}
	if (gamewe_lookup_setting(id, "global_chat", value, charsmax(value)) && equal(value, "0")) {
		g_gamewe_global_chat[id] = 0
	}
	// "start,end,kill"; only commands of gamewe_auto_commands are taken
	if (gamewe_lookup_setting(id, "auto", value, charsmax(value))) {
		new pos = 0
		for (new i = 0; i < GAMEWE_AUTO_EVENTS; i++) {
			new part[GAMEWE_AUTO_LENGTH], n = 0
			while (value[pos] != EOS && value[pos] != ',') {
				if (n < charsmax(part)) {
					part[n++] = value[pos]
				}
				pos++
			}
			part[n] = EOS
			if (value[pos] == ',') {
				pos++
			}
			if (gamewe_auto_command_index(part) >= 0) {
				copy(g_gamewe_auto[GAMEWE_AUTO_SLOT(id, i)], charsmax(g_gamewe_auto[]), part)
			}
		}
	}
	if (gamewe_lookup_setting(id, "lang", value, charsmax(value))) {
		g_gamewe_lang[id] = gamewe_lang_index(value)
	}
#if defined GAMEWE_DAMAGE_DISPLAY
	g_gamewe_player_damage_display[id] = GAMEWE_SETTING_DEFAULT
	if (gamewe_lookup_setting(id, "damage_display", value, charsmax(value))) {
		new setting = is_str_num(value) ? str_to_num(value) : GAMEWE_SETTING_DEFAULT
		if (setting >= 0 && setting <= 2) {
			g_gamewe_player_damage_display[id] = setting
		}
	}
#endif
}

stock gamewe_save_player_setting(id, const setting[], value)
{
	new key[MAX_AUTHID_LENGTH + 32]
	if (!gamewe_settings_key(id, setting, key, charsmax(key))) {
		return
	}
	if (value == GAMEWE_SETTING_DEFAULT) {
		nvault_remove(g_gamewe_settings_vault, key)
	} else {
		new text[8]
		num_to_str(value, text, charsmax(text))
		nvault_set(g_gamewe_settings_vault, key, text)
	}
}

stock gamewe_save_player_string(id, const setting[], const value[])
{
	new key[MAX_AUTHID_LENGTH + 32]
	if (!gamewe_settings_key(id, setting, key, charsmax(key))) {
		return
	}
	if (value[0] == EOS) {
		nvault_remove(g_gamewe_settings_vault, key)
	} else {
		nvault_set(g_gamewe_settings_vault, key, value)
	}
}

stock gamewe_save_auto_commands(id)
{
	new value[64]
	formatex(value, charsmax(value), "%s,%s,%s", g_gamewe_auto[GAMEWE_AUTO_SLOT(id, GAMEWE_AUTO_START)],
		g_gamewe_auto[GAMEWE_AUTO_SLOT(id, GAMEWE_AUTO_END)], g_gamewe_auto[GAMEWE_AUTO_SLOT(id, GAMEWE_AUTO_KILL)])
	if (equal(value, ",,")) {
		value[0] = EOS
	}
	gamewe_save_player_string(id, "auto", value)
}

// index in gamewe_auto_commands, -1 if the command cannot be an auto command
stock gamewe_auto_command_index(const command[])
{
	if (command[0] == EOS) {
		return -1
	}
	for (new i = 0; i < sizeof(gamewe_auto_commands); i++) {
		if (equali(command, gamewe_auto_commands[i])) {
			return i
		}
	}
	return -1
}


/*
 * Settings the plugin keeps: gamewe_display, gamewe_chat, gamewe_auto and
 * gamewe_lang (also gameme_*). Not logged, so the collector does not see them.
 */

// a chat line of the plugin in the player's language: the [gameWE] tag, or
// the message prefix in its place
stock gamewe_reply(id, const message[])
{
	new line[GAMEWE_MAX_CHAT]
	formatex(line, charsmax(line), "%s %s", GAMEWE_CHAT_TAG, message)
	gamewe_strip_chat_tag(line, charsmax(line))
	gamewe_mod_chat(id, line, 0, false)
}

// a message of the table in the player's language with up to three string
// arguments (the table's templates only hold %s)
stock gamewe_reply_msg(id, msg, const a1[] = "", const a2[] = "", const a3[] = "")
{
	new template[GAMEWE_MAX_CHAT], message[GAMEWE_MAX_CHAT]
	gamewe_msg(id, msg, template, charsmax(template))
	formatex(message, charsmax(message), template, a1, a2, a3)
	gamewe_reply(id, message)
}

// true if said is one of the settings commands
stock bool:gamewe_player_setting(client, const said[])
{
	new command[64], argument[64], argument2[64]
	new pos = 0
	gamewe_next_token(said, pos, command, charsmax(command))
	gamewe_next_token(said, pos, argument, charsmax(argument))
	gamewe_next_token(said, pos, argument2, charsmax(argument2))

	new start = (command[0] == '/' || command[0] == '!') ? 1 : 0
	if (!equali(command[start], "gamewe_", 7) && !equali(command[start], "gameme_", 7)) {
		return false
	}
	new name[16]
	copy(name, charsmax(name), command[start + 7])
	if (!equali(name, "display") && !equali(name, "chat") && !equali(name, "auto") && !equali(name, "lang")) {
		return false
	}
	// every change writes the nVault (gamewe_lang also a log line): at most
	// one per gamewe_chat_cooldown
	new Float:now = get_gametime()
	new Float:cooldown = get_pcvar_float(g_gamewe_chat_cooldown)
	if (cooldown > 0.0 && g_gamewe_last_trigger[client] > 0.0 && (now - g_gamewe_last_trigger[client]) < cooldown && now >= g_gamewe_last_trigger[client]) {
		return true
	}
	g_gamewe_last_trigger[client] = now

	if (equali(name, "display")) {
		gamewe_set_event_setting(client, false, argument)
	} else if (equali(name, "chat")) {
		gamewe_set_event_setting(client, true, argument)
	} else if (equali(name, "auto")) {
		gamewe_auto_command(client, argument, argument2)
	} else {
		gamewe_language_command(client, argument)
	}
	return true
}

// gamewe_display 0|1 (console events) or gamewe_chat 0|1 (global chat)
stock gamewe_set_event_setting(id, bool:chat, const value[])
{
	new setting
	if (equal(value, "1") || equali(value, "on")) {
		setting = 1
	} else if (equal(value, "0") || equali(value, "off")) {
		setting = 0
	} else {
		if (chat) {
			gamewe_reply_msg(id, GAMEWE_MSG_ONOFF_USAGE, "gamewe_chat", "gamewe_chat")
		} else {
			gamewe_reply_msg(id, GAMEWE_MSG_ONOFF_USAGE, "gamewe_display", "gamewe_display")
		}
		return
	}
	// stored only when off: on is the default
	new stored[2]
	if (setting == 0) {
		copy(stored, charsmax(stored), "0")
	}
	if (chat) {
		g_gamewe_global_chat[id] = setting
		gamewe_save_player_string(id, "global_chat", stored)
		gamewe_reply_msg(id, setting ? GAMEWE_MSG_CHAT_ON : GAMEWE_MSG_CHAT_OFF)
	} else {
		g_gamewe_console_events[id] = setting
		gamewe_save_player_string(id, "console_events", stored)
		gamewe_reply_msg(id, setting ? GAMEWE_MSG_DISPLAY_ON : GAMEWE_MSG_DISPLAY_OFF)
	}
}

// gamewe_auto clear | gamewe_auto start|end|kill [<command>|clear] | gamewe_auto
stock gamewe_auto_command(id, const type[], const command[])
{
	if (equali(type, "clear")) {
		for (new i = 0; i < GAMEWE_AUTO_EVENTS; i++) {
			g_gamewe_auto[GAMEWE_AUTO_SLOT(id, i)][0] = EOS
		}
		gamewe_save_auto_commands(id)
		gamewe_reply_msg(id, GAMEWE_MSG_AUTO_CLEARED)
		return
	}
	new auto_event = -1
	for (new i = 0; i < GAMEWE_AUTO_EVENTS; i++) {
		if (equali(type, gamewe_auto_types[i])) {
			auto_event = i
		}
	}
	if (auto_event < 0) {
		new off[32], current[GAMEWE_AUTO_EVENTS][GAMEWE_AUTO_LENGTH]
		gamewe_msg(id, GAMEWE_MSG_OFF, off, charsmax(off))
		for (new i = 0; i < GAMEWE_AUTO_EVENTS; i++) {
			if (g_gamewe_auto[GAMEWE_AUTO_SLOT(id, i)][0] != EOS) {
				copy(current[i], charsmax(current[]), g_gamewe_auto[GAMEWE_AUTO_SLOT(id, i)])
			} else {
				copy(current[i], charsmax(current[]), off)
			}
		}
		gamewe_reply_msg(id, GAMEWE_MSG_AUTO_LIST, current[GAMEWE_AUTO_START], current[GAMEWE_AUTO_END], current[GAMEWE_AUTO_KILL])
		gamewe_reply_msg(id, GAMEWE_MSG_AUTO_USAGE)
		return
	}
	new event_name[64]
	gamewe_msg(id, GAMEWE_MSG_AUTO_START + auto_event, event_name, charsmax(event_name))
	if (command[0] == EOS || equali(command, "clear")) {
		g_gamewe_auto[GAMEWE_AUTO_SLOT(id, auto_event)][0] = EOS
		gamewe_save_auto_commands(id)
		gamewe_reply_msg(id, GAMEWE_MSG_AUTO_REMOVED, event_name)
		return
	}
	new index = gamewe_auto_command_index(command)
	if (index < 0) {
		gamewe_reply_msg(id, GAMEWE_MSG_AUTO_UNKNOWN, command)
		gamewe_reply_msg(id, GAMEWE_MSG_AUTO_USAGE)
		return
	}
	copy(g_gamewe_auto[GAMEWE_AUTO_SLOT(id, auto_event)], charsmax(g_gamewe_auto[]), gamewe_auto_commands[index])
	gamewe_save_auto_commands(id)
	gamewe_reply_msg(id, GAMEWE_MSG_AUTO_SET, event_name, gamewe_auto_commands[index])
}

// gamewe_lang <code>: the language of the plugin's and the collector's messages
stock gamewe_language_command(id, const code[])
{
	new index = gamewe_lang_index(code)
	if (index < 0) {
		gamewe_reply_msg(id, GAMEWE_MSG_LANG_USAGE)
		return
	}
	g_gamewe_lang[id] = index
	gamewe_save_player_string(id, "lang", gamewe_lang_codes[index])
	gamewe_apply_language(id)
	gamewe_reply_msg(id, GAMEWE_MSG_LANG_SET, gamewe_lang_codes[index])
}

// tells the collector the player's chosen language
stock gamewe_apply_language(id)
{
	if (id >= 1 && id <= MaxClients && g_gamewe_lang[id] >= 0 && is_user_connected(id) && !is_user_bot(id)) {
		gamewe_log_player_setup(id, "gamewe_lang", gamewe_lang_codes[g_gamewe_lang[id]])
	}
}

// sends the auto command of the event for every player (only: just that one)
stock gamewe_run_auto(auto_event, only = 0)
{
	new Float:now = get_gametime()
	for (new id = 1; id <= MaxClients; id++) {
		if ((only > 0 && id != only) || !is_user_connected(id) || is_user_bot(id) || is_user_hltv(id) || g_gamewe_auto[GAMEWE_AUTO_SLOT(id, auto_event)][0] == EOS) {
			continue
		}
		new Float:last = g_gamewe_last_auto[id][auto_event]
		if (last > 0.0 && now >= last && (now - last) < 2.0) {
			continue
		}
		g_gamewe_last_auto[id][auto_event] = now
		new Float:cooldown = get_pcvar_float(g_gamewe_chat_cooldown)
		if (cooldown > 0.0 && g_gamewe_last_trigger[id] > 0.0 && (now - g_gamewe_last_trigger[id]) < cooldown && now >= g_gamewe_last_trigger[id]) {
			continue
		}
		g_gamewe_last_trigger[id] = now
		new command[GAMEWE_AUTO_LENGTH + 1]
		formatex(command, charsmax(command), "/%s", g_gamewe_auto[GAMEWE_AUTO_SLOT(id, auto_event)])
		gamewe_log_player_event(id, "say", command)
	}
}

public gamewe_auto_round_start()
{
	gamewe_run_auto(GAMEWE_AUTO_START)
}

public gamewe_auto_round_end()
{
	gamewe_run_auto(GAMEWE_AUTO_END)
}

public gamewe_auto_death()
{
	new victim = read_data(2)
	if (victim >= 1 && victim <= MaxClients) {
		gamewe_run_auto(GAMEWE_AUTO_KILL, victim)
	}
}

// draws "Console events" / "Global chat" with the player's current value
public gamewe_menu_events_callback(id, menu, item)
{
	new command[16], name[2], access, callback
	menu_item_getinfo(menu, item, access, command, charsmax(command), name, charsmax(name), callback)
	new bool:chat = bool:equal(command, "#global_chat")
	new value[32], text[64]
	gamewe_msg(id, (chat ? g_gamewe_global_chat[id] : g_gamewe_console_events[id]) ? GAMEWE_MSG_ON : GAMEWE_MSG_OFF, value, charsmax(value))
	new template[64]
	gamewe_msg(id, chat ? GAMEWE_MSG_CHAT_ENTRY : GAMEWE_MSG_CONSOLE_ENTRY, template, charsmax(template))
	formatex(text, charsmax(text), template, value)
	menu_item_setname(menu, item, text)
	return ITEM_ENABLED
}


#if defined GAMEWE_DAMAGE_DISPLAY
// the player's damage display mode (0 = off, 1 = menu, 2 = chat): the player's
// own choice, otherwise the server default gamewe_damage_display
stock gamewe_get_damage_display(id)
{
	if (get_pcvar_num(g_gamewe_damage_display_allow) != 1) {
		return 0
	}
	new mode = g_gamewe_player_damage_display[id]
	if (mode == GAMEWE_SETTING_DEFAULT) {
		mode = get_pcvar_num(g_gamewe_damage_display)
	}
	if (mode < 0 || mode > 2) {
		mode = 0
	}
	return mode
}

stock gamewe_damage_mode_name(mode, name[], len)
{
	switch (mode) {
		case 1:  copy(name, len, "menu")
		case 2:  copy(name, len, "chat")
		default: copy(name, len, "off")
	}
}

// draws the "Damage display" entry with the player's current value
public gamewe_menu_settings_callback(id, menu, item)
{
	new text[64], mode_name[16]
	if (get_pcvar_num(g_gamewe_damage_display_allow) != 1) {
		menu_item_setname(menu, item, "Damage display: disabled by the server")
		return ITEM_DISABLED
	}
	gamewe_damage_mode_name(gamewe_get_damage_display(id), mode_name, charsmax(mode_name))
	if (g_gamewe_player_damage_display[id] == GAMEWE_SETTING_DEFAULT) {
		formatex(text, charsmax(text), "Damage display: server default (%s)", mode_name)
	} else {
		formatex(text, charsmax(text), "Damage display: %s", mode_name)
	}
	menu_item_setname(menu, item, text)
	return ITEM_ENABLED
}
#endif


/*
 * gameWE menu: the entries send the chat command for the player
 */

stock gamewe_build_menus()
{
	g_gamewe_menu_main = menu_create("gameWE - Main Menu", "gamewe_menu_main_handle")
	menu_additem(g_gamewe_menu_main, "Display Rank",           "/rank")
	menu_additem(g_gamewe_menu_main, "Next Players",           "/next")
	menu_additem(g_gamewe_menu_main, "Top10 Players",          "/top10")
#if defined GAMEWE_FULL_MENU
	menu_additem(g_gamewe_menu_main, "Clans Ranking",          "/clans")
	menu_additem(g_gamewe_menu_main, "Server Status",          "/status")
	menu_additem(g_gamewe_menu_main, "Statsme",                "/statsme")
#endif
	menu_additem(g_gamewe_menu_main, "Settings",               "#settings")
#if defined GAMEWE_FULL_MENU
	menu_additem(g_gamewe_menu_main, "Weapon Usage",           "/weapons")
	menu_additem(g_gamewe_menu_main, "Weapons Accuracy",       "/accuracy")
	menu_additem(g_gamewe_menu_main, "Weapons Targets",        "/targets")
	menu_additem(g_gamewe_menu_main, "Player Kills",           "/kills")
	menu_additem(g_gamewe_menu_main, "VAC Cheaterlist",        "/cheaters")
	menu_additem(g_gamewe_menu_main, "Display Help",           "/help")
#endif

	// one ranking entry: it hides or shows the player, depending on the state
	// the collector sent last (gamewe_hidestate), and switches while the state
	// is not known; the collector answers with psay and the new state. The
	// display settings are kept by the plugin and show their current value.
	g_gamewe_menu_settings = menu_create("gameWE - Settings", "gamewe_menu_command_handle")
	menu_additem(g_gamewe_menu_settings, "Ranking", "#ranking", 0, menu_makecallback("gamewe_menu_ranking_callback"))
	menu_additem(g_gamewe_menu_settings, "Console events", "#console_events", 0, menu_makecallback("gamewe_menu_events_callback"))
	menu_additem(g_gamewe_menu_settings, "Global chat", "#global_chat", 0, menu_makecallback("gamewe_menu_events_callback"))
#if defined GAMEWE_DAMAGE_DISPLAY
	menu_additem(g_gamewe_menu_settings, "Damage display", "#damage_display", 0, menu_makecallback("gamewe_menu_settings_callback"))
#endif
	menu_setprop(g_gamewe_menu_settings, MPROP_PERPAGE, 0)
}

stock gamewe_display_menu(client, menu)
{
	if (is_user_connected(client) && !is_user_bot(client)) {
		menu_display(client, menu, 0)
	}
}

// the settings menu; without the ranking state yet the collector is asked
// for it (at most every 5 seconds, so the menu cannot flood the collector)
// and the menu is shown again when gamewe_hidestate arrives
stock gamewe_display_settings_menu(client)
{
	if (!is_user_connected(client) || is_user_bot(client)) {
		return
	}
	new Float:now = get_gametime()
	new Float:asked = g_gamewe_hideranking_asked[client]
	if (g_gamewe_hideranking[client] == GAMEWE_HIDERANKING_UNKNOWN && (asked <= 0.0 || (now - asked) >= 5.0 || now < asked)) {
		g_gamewe_hideranking_asked[client] = now
		g_gamewe_hideranking_wait[client] = true
		gamewe_log_player_request(client, "hideranking")
	}
	menu_display(client, g_gamewe_menu_settings, 0)
}

// draws the ranking entry for the player's current state
public gamewe_menu_ranking_callback(id, menu, item)
{
	new text[64]
	switch (g_gamewe_hideranking[id]) {
		case 0: gamewe_msg(id, GAMEWE_MSG_RANK_HIDE, text, charsmax(text))
		case 3: gamewe_msg(id, GAMEWE_MSG_RANK_SHOW, text, charsmax(text))
		case GAMEWE_HIDERANKING_UNKNOWN: gamewe_msg(id, GAMEWE_MSG_RANK_SWITCH, text, charsmax(text))
		default: {
			gamewe_msg(id, GAMEWE_MSG_RANK_LOCKED, text, charsmax(text))
			menu_item_setname(menu, item, text)
			return ITEM_DISABLED
		}
	}
	menu_item_setname(menu, item, text)
	return ITEM_ENABLED
}

public gamewe_menu_main_handle(id, menu, item)
{
	if (item < 0) {
		return PLUGIN_CONTINUE
	}
	new command[48], name[2], access, callback
	menu_item_getinfo(menu, item, access, command, charsmax(command), name, charsmax(name), callback)

	if (equal(command, "#settings")) {
		gamewe_display_settings_menu(id)
	} else if (command[0] == '/') {
		gamewe_log_player_event(id, "say", command)
	}
	return PLUGIN_HANDLED
}

public gamewe_menu_command_handle(id, menu, item)
{
	if (item < 0) {
		// closed by the player: do not open it again when the state arrives
		if (item == MENU_EXIT && id >= 1 && id <= MaxClients) {
			g_gamewe_hideranking_wait[id] = false
		}
		return PLUGIN_CONTINUE
	}
	new command[48], name[2], access, callback
	menu_item_getinfo(menu, item, access, command, charsmax(command), name, charsmax(name), callback)
	if (command[0] == '/') {
		gamewe_log_player_event(id, "say", command)
	}
	if (equal(command, "#ranking")) {
		switch (g_gamewe_hideranking[id]) {
			case 0: gamewe_log_player_event(id, "say", "/gamewe_hideranking 1")
			case 3: gamewe_log_player_event(id, "say", "/gamewe_hideranking 0")
			case GAMEWE_HIDERANKING_UNKNOWN: gamewe_log_player_event(id, "say", "/gamewe_hideranking")
		}
		// the collector answers with the new state: show the menu with it
		g_gamewe_hideranking_wait[id] = true
	}
	if (equal(command, "#console_events") || equal(command, "#global_chat")) {
		new bool:chat = bool:equal(command, "#global_chat")
		new on = chat ? g_gamewe_global_chat[id] : g_gamewe_console_events[id]
		if (on) {
			gamewe_set_event_setting(id, chat, "0")
		} else {
			gamewe_set_event_setting(id, chat, "1")
		}
		gamewe_display_menu(id, menu)
	}
#if defined GAMEWE_DAMAGE_DISPLAY
	if (equal(command, "#damage_display") && get_pcvar_num(g_gamewe_damage_display_allow) == 1) {
		// server default -> off -> menu -> chat -> server default
		new setting = g_gamewe_player_damage_display[id]
		setting = (setting >= 2) ? GAMEWE_SETTING_DEFAULT : (setting + 1)
		g_gamewe_player_damage_display[id] = setting
		gamewe_save_player_setting(id, "damage_display", setting)
		gamewe_display_menu(id, menu)
	}
#endif
	return PLUGIN_HANDLED
}
