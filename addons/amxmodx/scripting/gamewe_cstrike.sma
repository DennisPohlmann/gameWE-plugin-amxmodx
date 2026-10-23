/**
 * gameWE - AMX Mod X plugin for Counter-Strike 1.6 and Condition Zero
 *
 * Logs extended player statistics to the game server log for the gameWE
 * collector and shows its answers (rank, top10, session ...) in game.
 * Needs the modules cstrike, csx, fakemeta and hamsandwich.
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

#include <amxmodx>
#include <cstrike>
#include <csx>
#include <fakemeta>

#define GAMEWE_FULL_MENU
#define GAMEWE_WSTATS_SIZE STATSX_MAX_STATS
#include <gamewe_core>
#include <gamewe_xstats>

new g_gamewe_log_damage

// the engine kill line that is replaced by the one with positions
new g_gamewe_replaced_line[512]


public plugin_init()
{
	gamewe_core_init("cstrike")
	gamewe_xstats_init()

	g_gamewe_log_damage = create_cvar("gamewe_log_damage", "1", FCVAR_NONE,
		"Log an ^"attacked^" line for every bullet hit", true, 0.0, true, 1.0)

	register_event("DeathMsg", "gamewe_cs_death", "a", "1>0")
	register_event("Damage", "gamewe_cs_damage", "b", "2!0", "3=0", "4!0")
	register_logevent("gamewe_cs_round_end", 2, "1=Round_End")
	register_forward(FM_AlertMessage, "gamewe_cs_alert_message")
}


public gamewe_cs_round_end()
{
	gamewe_log_weaponstats_alive()
}


/*
 * Kills with positions and headshots
 *
 * DeathMsg is sent just before the game logs the kill. The plugin logs the
 * kill line with the positions and drops the plain line of the game in
 * gamewe_cs_alert_message(), so every kill is counted once.
 */
public gamewe_cs_death()
{
	new killer = read_data(1)
	new victim = read_data(2)
	new headshot = read_data(3)
	new weapon[32]
	read_data(4, weapon, charsmax(weapon))

	g_gamewe_replaced_line[0] = EOS
	if (killer < 1 || killer > MaxClients || !is_user_connected(killer)) {
		return
	}

	new killer_name[MAX_NAME_LENGTH], killer_authid[MAX_AUTHID_LENGTH], killer_team[16]
	new killer_userid = get_user_userid(killer)
	get_user_name(killer, killer_name, charsmax(killer_name))
	get_user_authid(killer, killer_authid, charsmax(killer_authid))
	get_user_team(killer, killer_team, charsmax(killer_team))
	new killer_origin[3]
	get_user_origin(killer, killer_origin)

	if (killer == victim) {
		formatex(g_gamewe_replaced_line, charsmax(g_gamewe_replaced_line), "^"%s<%d><%s><%s>^" committed suicide with ^"%s^"",
			killer_name, killer_userid, killer_authid, killer_team, weapon)
		log_message("%s (attacker_position ^"%d %d %d^")",
			g_gamewe_replaced_line, killer_origin[0], killer_origin[1], killer_origin[2])
		return
	}

	if (victim < 1 || victim > MaxClients || !is_user_connected(victim)) {
		return
	}

	new victim_name[MAX_NAME_LENGTH], victim_authid[MAX_AUTHID_LENGTH], victim_team[16]
	new victim_userid = get_user_userid(victim)
	get_user_name(victim, victim_name, charsmax(victim_name))
	get_user_authid(victim, victim_authid, charsmax(victim_authid))
	get_user_team(victim, victim_team, charsmax(victim_team))
	new victim_origin[3]
	get_user_origin(victim, victim_origin)

	formatex(g_gamewe_replaced_line, charsmax(g_gamewe_replaced_line), "^"%s<%d><%s><%s>^" killed ^"%s<%d><%s><%s>^" with ^"%s^"",
		killer_name, killer_userid, killer_authid, killer_team,
		victim_name, victim_userid, victim_authid, victim_team, weapon)
	log_message("%s (attacker_position ^"%d %d %d^") (victim_position ^"%d %d %d^")",
		g_gamewe_replaced_line,
		killer_origin[0], killer_origin[1], killer_origin[2],
		victim_origin[0], victim_origin[1], victim_origin[2])

	if (headshot > 0 && !equal(killer_team, victim_team)) {
		log_message("^"%s<%d><%s><%s>^" triggered ^"headshot^"", killer_name, killer_userid, killer_authid, killer_team)
	}
}

public gamewe_cs_alert_message(AlertType:type, const message[])
{
	if (type != at_logged || g_gamewe_replaced_line[0] == EOS) {
		return FMRES_IGNORED
	}
	// our own line has the positions, the line of the game does not
	if (contain(message, g_gamewe_replaced_line) != -1 && contain(message, "_position ^"") == -1) {
		g_gamewe_replaced_line[0] = EOS
		return FMRES_SUPERCEDE
	}
	return FMRES_IGNORED
}


/*
 * Bullet hits: "attacked" lines with damage and hit group
 */
public gamewe_cs_damage(victim)
{
	if (get_pcvar_num(g_gamewe_log_damage) <= 0 || victim < 1 || victim > MaxClients || !is_user_connected(victim)) {
		return PLUGIN_CONTINUE
	}

	new weapon_id, body
	new attacker = get_user_attacker(victim, weapon_id, body)
	if (attacker < 1 || attacker > MaxClients || attacker == victim || weapon_id <= 0 || !is_user_connected(attacker)) {
		return PLUGIN_CONTINUE
	}

	new damage_armor = read_data(1)
	new damage = read_data(2)

	new victim_name[MAX_NAME_LENGTH], victim_authid[MAX_AUTHID_LENGTH], victim_team[16]
	get_user_name(victim, victim_name, charsmax(victim_name))
	get_user_authid(victim, victim_authid, charsmax(victim_authid))
	get_user_team(victim, victim_team, charsmax(victim_team))

	new attacker_name[MAX_NAME_LENGTH], attacker_authid[MAX_AUTHID_LENGTH], attacker_team[16], weapon[32]
	get_user_name(attacker, attacker_name, charsmax(attacker_name))
	get_user_authid(attacker, attacker_authid, charsmax(attacker_authid))
	get_user_team(attacker, attacker_team, charsmax(attacker_team))
	xmod_get_wpnlogname(weapon_id, weapon, charsmax(weapon))

	new hitgroup[16]
	switch (body) {
		case HIT_GENERIC:  copy(hitgroup, charsmax(hitgroup), "generic")
		case HIT_HEAD:     copy(hitgroup, charsmax(hitgroup), "head")
		case HIT_CHEST:    copy(hitgroup, charsmax(hitgroup), "chest")
		case HIT_STOMACH:  copy(hitgroup, charsmax(hitgroup), "stomach")
		case HIT_LEFTARM:  copy(hitgroup, charsmax(hitgroup), "left arm")
		case HIT_RIGHTARM: copy(hitgroup, charsmax(hitgroup), "right arm")
		case HIT_LEFTLEG:  copy(hitgroup, charsmax(hitgroup), "left leg")
		case HIT_RIGHTLEG: copy(hitgroup, charsmax(hitgroup), "right leg")
		case HIT_SHIELD:   copy(hitgroup, charsmax(hitgroup), "shield")
		default:           copy(hitgroup, charsmax(hitgroup), "unknown")
	}

	log_message("^"%s<%d><%s><%s>^" attacked ^"%s<%d><%s><%s>^" with ^"%s^" (damage ^"%d^") (damage_armor ^"%d^") (health ^"%d^") (armor ^"%d^") (hitgroup ^"%s^")",
		attacker_name, get_user_userid(attacker), attacker_authid, attacker_team,
		victim_name, get_user_userid(victim), victim_authid, victim_team,
		weapon, damage, damage_armor, max(get_user_health(victim), 0), get_user_armor(victim), hitgroup)
	return PLUGIN_CONTINUE
}


/*
 * Colored chat
 *
 * colored 1: the tags x01 (default), x03 (team color) and x04 (green) become
 * color codes. The first player name in the message gets the color of that
 * player's team, other names are green. Without a name, "TERRORIST" or "CT"
 * get their team color.
 */
stock gamewe_cs_color_names(message[], len)
{
	new first_pos = len, first_player = 0
	new players_in_message[MAX_PLAYERS], count = 0
	new name[MAX_NAME_LENGTH]

	for (new player = 1; player <= MaxClients; player++) {
		if (!is_user_connected(player)) {
			continue
		}
		get_user_name(player, name, charsmax(name))
		if (strlen(name) < 2) {
			continue
		}
		new pos = contain(message, name)
		if (pos > -1) {
			players_in_message[count++] = player
			if (pos < first_pos) {
				first_pos = pos
				first_player = player
			}
		}
	}

	new colored_name[MAX_NAME_LENGTH + 2]
	for (new i = 0; i < count; i++) {
		get_user_name(players_in_message[i], name, charsmax(name))
		formatex(colored_name, charsmax(colored_name), "%s%s^1", (players_in_message[i] == first_player) ? "^3" : "^4", name)
		replace_string(message, len, name, colored_name)
	}
	return first_player
}

stock gamewe_cs_color_team(message[], len)
{
	if (contain(message, "TERRORIST") > -1) {
		replace_string(message, len, "TERRORIST", "^3TERRORIST^1")
		return print_team_red
	}
	if (contain(message, "CT") > -1) {
		replace_string(message, len, "CT", "^3CT^1")
		return print_team_blue
	}
	return print_team_default
}

stock gamewe_mod_chat(player, const message[], colored, bool:green)
{
	new text[GAMEWE_MAX_CHAT]
	copy(text, charsmax(text), message)

	new sender = print_team_default
	if (green) {
		gamewe_remove_color_tags(text, charsmax(text))
		format(text, charsmax(text), "^4%s", text)
	} else if (colored == 1) {
		sender = gamewe_cs_color_names(text, charsmax(text))
		if (sender == 0) {
			sender = gamewe_cs_color_team(text, charsmax(text))
		}
		replace_string(text, charsmax(text), "x01", "^1")
		replace_string(text, charsmax(text), "x03", "^3")
		replace_string(text, charsmax(text), "x04", "^4")
		gamewe_remove_color_tags(text, charsmax(text))
	} else {
		gamewe_remove_color_tags(text, charsmax(text))
	}

	new prefix[64]
	gamewe_get_prefix(prefix, charsmax(prefix))
	if (prefix[0] != EOS) {
		client_print_color(player, sender, "^4%s^1 %s", prefix, text)
	} else {
		client_print_color(player, sender, "^1%s", text)
	}
}


/*
 * Team swap for team balancers
 */
stock gamewe_mod_swap(player)
{
	new CsTeams:team = cs_get_user_team(player)
	new CsTeams:new_team
	new team_name[16]
	if (team == CS_TEAM_T) {
		new_team = CS_TEAM_CT
		copy(team_name, charsmax(team_name), "CT")
	} else if (team == CS_TEAM_CT) {
		new_team = CS_TEAM_T
		copy(team_name, charsmax(team_name), "TERRORIST")
	} else {
		return
	}

	// the game does not log a team change made by a plugin
	gamewe_log_player_event(player, "joined team", team_name)
	cs_set_user_team(player, new_team)
	if (is_user_alive(player)) {
		cs_reset_user_model(player)
	}
}
