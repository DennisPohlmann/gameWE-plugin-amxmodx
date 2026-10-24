/**
 * gameWE - AMX Mod X plugin, weapon statistics
 *
 * Writes the "weaponstats" and "weaponstats2" lines from the stats module
 * of the mod (csx, dodx, tfcx). Include it after the module include and set
 * GAMEWE_WSTATS_SIZE to the size of the stats array of get_user_wstats().
 *
 * Also the damage display: the damage a player did and received during the
 * current life, per opponent, shown on death (menu or chat, the player's
 * setting in the gameWE settings menu) and, in menu mode, at round end.
 * Needs GAMEWE_DAMAGE_DISPLAY defined before gamewe_core.inc is included.
 *
 * Fork of the gameME Plugin 2.4 for AMX Mod X
 * (https://github.com/gamemedev/plugin-amxmodx),
 * Copyright (C) 2007-2017 TTS Oetzel & Goerz GmbH.
 * Copyright (C) 2026 gameWE contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#if defined _gamewe_xstats_included
	#endinput
#endif
#define _gamewe_xstats_included

#include <hamsandwich>

#if !defined GAMEWE_DAMAGE_DISPLAY
	#error "define GAMEWE_DAMAGE_DISPLAY before including gamewe_core"
#endif

#if !defined GAMEWE_WSTATS_SIZE
	#define GAMEWE_WSTATS_SIZE 8
#endif

// order of the stats array, the same in csx, dodx and tfcx
#define GAMEWE_WS_KILLS      0
#define GAMEWE_WS_DEATHS     1
#define GAMEWE_WS_HEADSHOTS  2
#define GAMEWE_WS_TEAMKILLS  3
#define GAMEWE_WS_SHOTS      4
#define GAMEWE_WS_HITS       5
#define GAMEWE_WS_DAMAGE     6

// set when the stats of the current life are logged, so they are never logged twice
new bool:g_gamewe_wstats_logged[MAX_PLAYERS + 1]

// damage of the current life for the damage display: [player][opponent]
new g_gamewe_given_hits[MAX_PLAYERS + 1][MAX_PLAYERS + 1]
new g_gamewe_given_damage[MAX_PLAYERS + 1][MAX_PLAYERS + 1]
new bool:g_gamewe_given_kill[MAX_PLAYERS + 1][MAX_PLAYERS + 1]
new g_gamewe_taken_hits[MAX_PLAYERS + 1][MAX_PLAYERS + 1]
new g_gamewe_taken_damage[MAX_PLAYERS + 1][MAX_PLAYERS + 1]
new g_gamewe_killer[MAX_PLAYERS + 1]

stock gamewe_xstats_init()
{
	RegisterHam(Ham_Spawn, "player", "gamewe_player_spawned", 1)
}

public gamewe_player_spawned(id)
{
	if (id >= 1 && id <= MaxClients && is_user_alive(id)) {
		reset_user_wstats(id)
		g_gamewe_wstats_logged[id] = false
		gamewe_damage_reset(id)
	}
}

stock gamewe_log_weaponstats(id)
{
	if (id < 1 || id > MaxClients || !is_user_connected(id) || g_gamewe_wstats_logged[id]) {
		return
	}
	g_gamewe_wstats_logged[id] = true

	new stats[GAMEWE_WSTATS_SIZE], hits[MAX_BODYHITS]
	new name[MAX_NAME_LENGTH], authid[MAX_AUTHID_LENGTH], team[16], weapon[32]
	new userid = get_user_userid(id)
	get_user_name(id, name, charsmax(name))
	get_user_authid(id, authid, charsmax(authid))
	get_user_team(id, team, charsmax(team))

	new max_weapons = xmod_get_maxweapons()
	for (new i = 1; i < max_weapons; i++) {
		if (!get_user_wstats(id, i, stats, hits)) {
			continue
		}
		// the log name is the weapon name of the kill lines
		xmod_get_wpnlogname(i, weapon, charsmax(weapon))
		if (weapon[0] == EOS) {
			continue
		}
		log_message("^"%s<%d><%s><%s>^" triggered ^"weaponstats^" (weapon ^"%s^") (shots ^"%d^") (hits ^"%d^") (kills ^"%d^") (headshots ^"%d^") (tks ^"%d^") (damage ^"%d^") (deaths ^"%d^")",
			name, userid, authid, team, weapon,
			stats[GAMEWE_WS_SHOTS], stats[GAMEWE_WS_HITS], stats[GAMEWE_WS_KILLS], stats[GAMEWE_WS_HEADSHOTS],
			stats[GAMEWE_WS_TEAMKILLS], stats[GAMEWE_WS_DAMAGE], stats[GAMEWE_WS_DEATHS])
		if (stats[GAMEWE_WS_HITS] > 0) {
			log_message("^"%s<%d><%s><%s>^" triggered ^"weaponstats2^" (weapon ^"%s^") (head ^"%d^") (chest ^"%d^") (stomach ^"%d^") (leftarm ^"%d^") (rightarm ^"%d^") (leftleg ^"%d^") (rightleg ^"%d^")",
				name, userid, authid, team, weapon,
				hits[HIT_HEAD], hits[HIT_CHEST], hits[HIT_STOMACH], hits[HIT_LEFTARM],
				hits[HIT_RIGHTARM], hits[HIT_LEFTLEG], hits[HIT_RIGHTLEG])
		}
	}
}

// stats of everybody who is still alive, at the end of a round
stock gamewe_log_weaponstats_alive()
{
	for (new i = 1; i <= MaxClients; i++) {
		if (is_user_alive(i)) {
			gamewe_log_weaponstats(i)
		}
	}
}

// forward of csx, dodx and tfcx
public client_death(killer, victim, wpnindex, hitplace, TK)
{
	gamewe_log_weaponstats(victim)
	gamewe_damage_death(killer, victim)
}

public client_disconnected(id, bool:drop, message[], maxlen)
{
	gamewe_log_weaponstats(id)
	g_gamewe_wstats_logged[id] = false
	gamewe_damage_reset(id)
	gamewe_damage_forget(id)
}


/*
 * Damage display
 */

stock gamewe_damage_reset(id)
{
	if (id < 1 || id > MaxClients) {
		return
	}
	for (new i = 1; i <= MaxClients; i++) {
		g_gamewe_given_hits[id][i] = 0
		g_gamewe_given_damage[id][i] = 0
		g_gamewe_given_kill[id][i] = false
		g_gamewe_taken_hits[id][i] = 0
		g_gamewe_taken_damage[id][i] = 0
	}
	g_gamewe_killer[id] = 0
}

// removes a player who left from the summaries of the others, so the next
// player in the slot does not inherit the entries
stock gamewe_damage_forget(id)
{
	if (id < 1 || id > MaxClients) {
		return
	}
	for (new i = 1; i <= MaxClients; i++) {
		g_gamewe_given_hits[i][id] = 0
		g_gamewe_given_damage[i][id] = 0
		g_gamewe_given_kill[i][id] = false
		g_gamewe_taken_hits[i][id] = 0
		g_gamewe_taken_damage[i][id] = 0
		if (g_gamewe_killer[i] == id) {
			g_gamewe_killer[i] = 0
		}
	}
}

// forward of csx, dodx and tfcx
public client_damage(attacker, victim, damage, wpnindex, hitplace, TA)
{
	if (attacker < 1 || attacker > MaxClients || victim < 1 || victim > MaxClients || attacker == victim || damage <= 0) {
		return
	}
	if (get_pcvar_num(g_gamewe_damage_display_allow) != 1) {
		return
	}
	g_gamewe_given_hits[attacker][victim]++
	g_gamewe_given_damage[attacker][victim] += damage
	g_gamewe_taken_hits[victim][attacker]++
	g_gamewe_taken_damage[victim][attacker] += damage
}

stock gamewe_damage_death(killer, victim)
{
	if (victim < 1 || victim > MaxClients) {
		return
	}
	if (killer >= 1 && killer <= MaxClients && killer != victim) {
		g_gamewe_given_kill[killer][victim] = true
		g_gamewe_killer[victim] = killer
	}
	if (!is_user_connected(victim) || is_user_bot(victim)) {
		return
	}
	switch (gamewe_get_damage_display(victim)) {
		case 1: gamewe_damage_menu(victim)
		case 2: gamewe_damage_chat(victim)
	}
}

// summary for everybody alive who wants the menu, at the end of a round
stock gamewe_damage_round_end()
{
	for (new i = 1; i <= MaxClients; i++) {
		if (is_user_alive(i) && !is_user_bot(i) && gamewe_get_damage_display(i) == 1) {
			gamewe_damage_menu(i)
		}
	}
}

// "Name - 54 dmg, 2 hits"
stock gamewe_damage_entry(opponent, damage, hits, entry[], len)
{
	new name[MAX_NAME_LENGTH]
	get_user_name(opponent, name, charsmax(name))
	if (hits == 1) {
		formatex(entry, len, "%s - %d dmg, 1 hit", name, damage)
	} else {
		formatex(entry, len, "%s - %d dmg, %d hits", name, damage, hits)
	}
}

stock gamewe_damage_menu(id)
{
	new text[GAMEWE_MAX_MENU], entry[96]
	// room for the last line "0. Close"
	new max_len = charsmax(text) - 16
	new len = copy(text, charsmax(text), "gameWE - Damage^n")
	new count = 0

	for (new pass = 0; pass < 2; pass++) {
		new bool:header = false
		for (new i = 1; i <= MaxClients; i++) {
			new hits = (pass == 0) ? g_gamewe_given_hits[id][i] : g_gamewe_taken_hits[id][i]
			if (hits <= 0 || !is_user_connected(i)) {
				continue
			}
			if (!header) {
				header = true
				if (len + 20 < max_len) {
					len += copy(text[len], charsmax(text) - len, (pass == 0) ? "^nDamage given:^n" : "^nDamage taken:^n")
				}
			}
			if (pass == 0) {
				gamewe_damage_entry(i, g_gamewe_given_damage[id][i], hits, entry, charsmax(entry))
				if (g_gamewe_given_kill[id][i]) {
					add(entry, charsmax(entry), " (killed)")
				}
			} else {
				gamewe_damage_entry(i, g_gamewe_taken_damage[id][i], hits, entry, charsmax(entry))
				if (g_gamewe_killer[id] == i) {
					add(entry, charsmax(entry), " (killer)")
				}
			}
			if (len + strlen(entry) + 4 < max_len) {
				len += formatex(text[len], charsmax(text) - len, "  %s^n", entry)
				count++
			}
		}
	}

	if (count > 0) {
		add(text, charsmax(text), "^n0. Close")
		show_menu(id, MENU_KEY_0, text, 15, "gameWE Damage")
	}
}

stock gamewe_damage_chat(id)
{
	new text[GAMEWE_MAX_CHAT], entry[96]
	// the prefix and the color codes need room as well
	new max_len = charsmax(text) - 48

	for (new pass = 0; pass < 2; pass++) {
		new len = copy(text, charsmax(text), (pass == 0) ? "Damage given: " : "Damage taken: ")
		new count = 0
		for (new i = 1; i <= MaxClients; i++) {
			new hits = (pass == 0) ? g_gamewe_given_hits[id][i] : g_gamewe_taken_hits[id][i]
			if (hits <= 0 || !is_user_connected(i)) {
				continue
			}
			if (pass == 0) {
				gamewe_damage_entry(i, g_gamewe_given_damage[id][i], hits, entry, charsmax(entry))
				if (g_gamewe_given_kill[id][i]) {
					add(entry, charsmax(entry), " (killed)")
				}
			} else {
				gamewe_damage_entry(i, g_gamewe_taken_damage[id][i], hits, entry, charsmax(entry))
				if (g_gamewe_killer[id] == i) {
					add(entry, charsmax(entry), " (killer)")
				}
			}
			if (len + strlen(entry) + 2 >= max_len) {
				// no room for more opponents
				len += copy(text[len], charsmax(text) - len, " ...")
				count++
				break
			}
			if (count > 0) {
				len += copy(text[len], charsmax(text) - len, "; ")
			}
			len += copy(text[len], charsmax(text) - len, entry)
			count++
		}
		if (count > 0) {
			gamewe_mod_chat(id, text, 0, false)
		}
	}
}
