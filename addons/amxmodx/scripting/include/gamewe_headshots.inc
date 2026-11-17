/**
 * gameWE - AMX Mod X plugin, headshots from the hitgroup
 *
 * For mods whose DeathMsg has no headshot flag (Day of Defeat, Half-Life
 * Deathmatch): the hitgroup of the last bullet that hit the victim decides,
 * logged as `triggered "headshot"` with the kill. Needs fakemeta and
 * hamsandwich. Call gamewe_headshots_init() from plugin_init.
 *
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

#if defined _gamewe_headshots_included
	#endinput
#endif
#define _gamewe_headshots_included

#include <fakemeta>
#include <hamsandwich>

// hitgroup of the last TraceAttack on the player, 0 after the damage
new g_gamewe_last_hitgroup[MAX_PLAYERS + 1]

stock gamewe_headshots_init()
{
	RegisterHam(Ham_TraceAttack, "player", "gamewe_hs_trace_attack", 1)
	RegisterHam(Ham_TakeDamage, "player", "gamewe_hs_take_damage", 1)
	register_event("DeathMsg", "gamewe_hs_death", "a", "1>0")
}

// TraceAttack comes before the damage, DeathMsg is sent within TakeDamage
// and the post hook of TakeDamage after it: a grenade kill does not reuse
// an older hitgroup.
public gamewe_hs_trace_attack(victim, attacker, Float:damage, Float:direction[3], trace, damagebits)
{
	if (victim >= 1 && victim <= MaxClients) {
		g_gamewe_last_hitgroup[victim] = get_tr2(trace, TR_iHitgroup)
	}
}

public gamewe_hs_take_damage(victim, inflictor, attacker, Float:damage, damagebits)
{
	if (victim >= 1 && victim <= MaxClients) {
		g_gamewe_last_hitgroup[victim] = 0
	}
}

// not for suicides and teamkills (team 0: no teams, as Half-Life Deathmatch
// without teamplay)
public gamewe_hs_death()
{
	new killer = read_data(1)
	new victim = read_data(2)
	if (killer > MaxClients || victim < 1 || victim > MaxClients || killer == victim ||
	    g_gamewe_last_hitgroup[victim] != HIT_HEAD || !is_user_connected(killer) || !is_user_connected(victim)) {
		return
	}
	new team = get_user_team(killer)
	if (team == 0 || team != get_user_team(victim)) {
		gamewe_log_player_event(killer, "triggered", "headshot")
	}
}
