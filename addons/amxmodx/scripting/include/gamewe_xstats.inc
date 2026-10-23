/**
 * gameWE - AMX Mod X plugin, weapon statistics
 *
 * Writes the "weaponstats" and "weaponstats2" lines from the stats module
 * of the mod (csx, dodx, tfcx). Include it after the module include and set
 * GAMEWE_WSTATS_SIZE to the size of the stats array of get_user_wstats().
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

stock gamewe_xstats_init()
{
	RegisterHam(Ham_Spawn, "player", "gamewe_player_spawned", 1)
}

public gamewe_player_spawned(id)
{
	if (id >= 1 && id <= MaxClients && is_user_alive(id)) {
		reset_user_wstats(id)
		g_gamewe_wstats_logged[id] = false
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
}

public client_disconnected(id, bool:drop, message[], maxlen)
{
	gamewe_log_weaponstats(id)
	g_gamewe_wstats_logged[id] = false
}
