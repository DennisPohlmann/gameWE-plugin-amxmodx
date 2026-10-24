/**
 * gameWE - AMX Mod X plugin for Day of Defeat
 *
 * Logs extended player statistics to the game server log for the gameWE
 * collector and shows its answers (rank, top10, session ...) in game.
 * Needs the modules dodx, dodfun, hamsandwich and nvault.
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
#include <dodx>
#include <dodfun>

#define GAMEWE_FULL_MENU
#define GAMEWE_DAMAGE_DISPLAY
#define GAMEWE_WSTATS_SIZE DODX_MAX_STATS
#include <gamewe_core>
#include <gamewe_xstats>


public plugin_init()
{
	gamewe_core_init("dod")
	gamewe_xstats_init()
	register_logevent("gamewe_dod_round_end", 2, "1=Round_End")
}


public gamewe_dod_round_end()
{
	gamewe_log_weaponstats_alive()
	gamewe_damage_round_end()
}


stock gamewe_mod_chat(player, const message[], colored, bool:green)
{
	#pragma unused colored, green
	new text[GAMEWE_MAX_CHAT]
	copy(text, charsmax(text), message)
	gamewe_remove_color_tags(text, charsmax(text))
	gamewe_plain_chat(player, text)
}


// team swap for team balancers
stock gamewe_mod_swap(player)
{
	new team = get_user_team(player)
	if (team == ALLIES) {
		// the game does not log a team change made by a plugin
		gamewe_log_player_event(player, "joined team", "Axis")
		dod_set_user_team(player, AXIS)
	} else if (team == AXIS) {
		gamewe_log_player_event(player, "joined team", "Allies")
		dod_set_user_team(player, ALLIES)
	}
}
