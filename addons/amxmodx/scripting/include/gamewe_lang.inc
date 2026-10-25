/**
 * gameWE - AMX Mod X plugin, messages in the players' languages
 *
 * The plugin's own chat lines and settings menu entries in the 9 languages
 * of gamewe_lang (the collector answers in the same languages). A player's
 * language is the gamewe_lang choice, else the game's (setinfo "lang"),
 * else English. The templates only hold %s; the order of the arguments is
 * the same in every language.
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

#if defined _gamewe_lang_included
	#endinput
#endif
#define _gamewe_lang_included

// the codes of gamewe_lang, in the order of the message tables (dk is Danish, also "da")
#define GAMEWE_LANGUAGES   9
#define GAMEWE_LANG_UNSET  -1
stock const gamewe_lang_codes[GAMEWE_LANGUAGES][] = { "en", "de", "ru", "es", "fr", "pt", "nl", "dk", "pl" }

// the player's gamewe_lang (index in gamewe_lang_codes), GAMEWE_LANG_UNSET if none
new g_gamewe_lang[MAX_PLAYERS + 1] = { GAMEWE_LANG_UNSET, ... }

enum {
	GAMEWE_MSG_ON,
	GAMEWE_MSG_OFF,
	GAMEWE_MSG_CONSOLE_ENTRY,
	GAMEWE_MSG_CHAT_ENTRY,
	GAMEWE_MSG_ONOFF_USAGE,
	GAMEWE_MSG_DISPLAY_ON,
	GAMEWE_MSG_DISPLAY_OFF,
	GAMEWE_MSG_CHAT_ON,
	GAMEWE_MSG_CHAT_OFF,
	GAMEWE_MSG_AUTO_START,
	GAMEWE_MSG_AUTO_END,
	GAMEWE_MSG_AUTO_KILL,
	GAMEWE_MSG_AUTO_SET,
	GAMEWE_MSG_AUTO_REMOVED,
	GAMEWE_MSG_AUTO_CLEARED,
	GAMEWE_MSG_AUTO_LIST,
	GAMEWE_MSG_AUTO_USAGE,
	GAMEWE_MSG_AUTO_UNKNOWN,
	GAMEWE_MSG_LANG_SET,
	GAMEWE_MSG_LANG_USAGE,
	GAMEWE_MSG_RANK_HIDE,
	GAMEWE_MSG_RANK_SHOW,
	GAMEWE_MSG_RANK_SWITCH,
	GAMEWE_MSG_RANK_LOCKED,
	GAMEWE_MESSAGES
}

stock const gamewe_msg_en[GAMEWE_MESSAGES][] = {
	"on",
	"off",
	"Console events: %s",
	"Global chat: %s",
	"Use %s 1 (on) or %s 0 (off).",
	"Console events on: the points of your kills and actions are shown in the console.",
	"Console events off.",
	"Global chat events on.",
	"Global chat events off.",
	"round start",
	"round end",
	"your death",
	"At %s, %s is now sent for you.",
	"No auto command at %s any more.",
	"Your auto commands are cleared.",
	"Auto commands: round start %s, round end %s, death %s.",
	"Use gamewe_auto start|end|kill <command> or gamewe_auto clear. Commands: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s cannot be an auto command.",
	"Language: %s.",
	"Use gamewe_lang en, de, ru, es, fr, pt, nl, dk or pl.",
	"Hide me in the ranking",
	"Show me in the ranking",
	"Hide or show me in the ranking",
	"Hidden from the ranking (only an admin can change it)"
}

stock const gamewe_msg_de[GAMEWE_MESSAGES][] = {
	"an",
	"aus",
	"Konsolen-Ereignisse: %s",
	"Globaler Chat: %s",
	"Nutze %s 1 (an) oder %s 0 (aus).",
	"Konsolen-Ereignisse an: Die Punkte deiner Kills und Aktionen erscheinen in der Konsole.",
	"Konsolen-Ereignisse aus.",
	"Globale Chat-Ereignisse an.",
	"Globale Chat-Ereignisse aus.",
	"Rundenstart",
	"Rundenende",
	"deinem Tod",
	"Bei %s wird jetzt %s für dich gesendet.",
	"Bei %s wird kein Befehl mehr gesendet.",
	"Deine automatischen Befehle sind gelöscht.",
	"Automatische Befehle: Rundenstart %s, Rundenende %s, Tod %s.",
	"Nutze gamewe_auto start|end|kill <Befehl> oder gamewe_auto clear. Befehle: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s kann kein automatischer Befehl sein.",
	"Sprache: %s.",
	"Nutze gamewe_lang en, de, ru, es, fr, pt, nl, dk oder pl.",
	"Mich in der Rangliste verstecken",
	"Mich in der Rangliste zeigen",
	"Mich in der Rangliste verstecken oder zeigen",
	"In der Rangliste versteckt (nur ein Admin kann das ändern)"
}

stock const gamewe_msg_ru[GAMEWE_MESSAGES][] = {
	"вкл",
	"выкл",
	"События в консоли: %s",
	"Общий чат: %s",
	"Используй %s 1 (вкл) или %s 0 (выкл).",
	"События в консоли включены: очки за убийства и действия выводятся в консоль.",
	"События в консоли выключены.",
	"События в общем чате включены.",
	"События в общем чате выключены.",
	"начало раунда",
	"конец раунда",
	"твоя смерть",
	"%s: теперь за тебя отправляется %s.",
	"%s: автокоманды больше нет.",
	"Твои автокоманды удалены.",
	"Автокоманды: начало раунда %s, конец раунда %s, смерть %s.",
	"Используй gamewe_auto start|end|kill <команда> или gamewe_auto clear. Команды: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s не может быть автокомандой.",
	"Язык: %s.",
	"Используй gamewe_lang en, de, ru, es, fr, pt, nl, dk или pl.",
	"Скрыть меня в рейтинге",
	"Показать меня в рейтинге",
	"Скрыть или показать меня в рейтинге",
	"Скрыт из рейтинга (изменить может только админ)"
}

stock const gamewe_msg_es[GAMEWE_MESSAGES][] = {
	"activado",
	"desactivado",
	"Eventos en consola: %s",
	"Chat global: %s",
	"Usa %s 1 (activar) o %s 0 (desactivar).",
	"Eventos en consola activados: los puntos de tus bajas y acciones aparecen en la consola.",
	"Eventos en consola desactivados.",
	"Eventos del chat global activados.",
	"Eventos del chat global desactivados.",
	"inicio de ronda",
	"fin de ronda",
	"tu muerte",
	"En %s ahora se envía %s por ti.",
	"Ya no hay comando automático en %s.",
	"Tus comandos automáticos se han borrado.",
	"Comandos automáticos: inicio de ronda %s, fin de ronda %s, muerte %s.",
	"Usa gamewe_auto start|end|kill <comando> o gamewe_auto clear. Comandos: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s no puede ser un comando automático.",
	"Idioma: %s.",
	"Usa gamewe_lang en, de, ru, es, fr, pt, nl, dk o pl.",
	"Ocultarme en la clasificación",
	"Mostrarme en la clasificación",
	"Ocultarme o mostrarme en la clasificación",
	"Oculto de la clasificación (solo un admin puede cambiarlo)"
}

stock const gamewe_msg_fr[GAMEWE_MESSAGES][] = {
	"activé",
	"désactivé",
	"Événements console : %s",
	"Chat global : %s",
	"Utilise %s 1 (activé) ou %s 0 (désactivé).",
	"Événements console activés : les points de tes kills et actions s'affichent dans la console.",
	"Événements console désactivés.",
	"Événements du chat global activés.",
	"Événements du chat global désactivés.",
	"début de manche",
	"fin de manche",
	"ta mort",
	"À %s, %s est maintenant envoyé pour toi.",
	"Plus de commande automatique à %s.",
	"Tes commandes automatiques sont effacées.",
	"Commandes automatiques : début de manche %s, fin de manche %s, mort %s.",
	"Utilise gamewe_auto start|end|kill <commande> ou gamewe_auto clear. Commandes : rank, kpd, session, next, top5, top10, top20, statsme",
	"%s ne peut pas être une commande automatique.",
	"Langue : %s.",
	"Utilise gamewe_lang en, de, ru, es, fr, pt, nl, dk ou pl.",
	"Me masquer dans le classement",
	"M'afficher dans le classement",
	"Me masquer ou m'afficher dans le classement",
	"Masqué du classement (seul un admin peut le changer)"
}

stock const gamewe_msg_pt[GAMEWE_MESSAGES][] = {
	"ligado",
	"desligado",
	"Eventos no console: %s",
	"Chat global: %s",
	"Use %s 1 (ligado) ou %s 0 (desligado).",
	"Eventos no console ligados: os pontos dos seus abates e ações aparecem no console.",
	"Eventos no console desligados.",
	"Eventos do chat global ligados.",
	"Eventos do chat global desligados.",
	"início da rodada",
	"fim da rodada",
	"sua morte",
	"Em %s, %s agora é enviado por você.",
	"Não há mais comando automático em %s.",
	"Seus comandos automáticos foram apagados.",
	"Comandos automáticos: início da rodada %s, fim da rodada %s, morte %s.",
	"Use gamewe_auto start|end|kill <comando> ou gamewe_auto clear. Comandos: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s não pode ser um comando automático.",
	"Idioma: %s.",
	"Use gamewe_lang en, de, ru, es, fr, pt, nl, dk ou pl.",
	"Ocultar-me no ranking",
	"Mostrar-me no ranking",
	"Ocultar ou mostrar-me no ranking",
	"Oculto do ranking (só um admin pode mudar)"
}

stock const gamewe_msg_nl[GAMEWE_MESSAGES][] = {
	"aan",
	"uit",
	"Console-gebeurtenissen: %s",
	"Globale chat: %s",
	"Gebruik %s 1 (aan) of %s 0 (uit).",
	"Console-gebeurtenissen aan: de punten van je kills en acties staan in de console.",
	"Console-gebeurtenissen uit.",
	"Globale chatgebeurtenissen aan.",
	"Globale chatgebeurtenissen uit.",
	"rondestart",
	"ronde-einde",
	"je dood",
	"Bij %s wordt nu %s voor je verstuurd.",
	"Geen automatisch commando meer bij %s.",
	"Je automatische commando's zijn gewist.",
	"Automatische commando's: rondestart %s, ronde-einde %s, dood %s.",
	"Gebruik gamewe_auto start|end|kill <commando> of gamewe_auto clear. Commando's: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s kan geen automatisch commando zijn.",
	"Taal: %s.",
	"Gebruik gamewe_lang en, de, ru, es, fr, pt, nl, dk of pl.",
	"Verberg mij in de ranglijst",
	"Toon mij in de ranglijst",
	"Verberg of toon mij in de ranglijst",
	"Verborgen in de ranglijst (alleen een admin kan dit wijzigen)"
}

stock const gamewe_msg_dk[GAMEWE_MESSAGES][] = {
	"til",
	"fra",
	"Konsolhændelser: %s",
	"Global chat: %s",
	"Brug %s 1 (til) eller %s 0 (fra).",
	"Konsolhændelser til: point for dine drab og handlinger vises i konsollen.",
	"Konsolhændelser fra.",
	"Globale chathændelser til.",
	"Globale chathændelser fra.",
	"rundestart",
	"rundeslut",
	"din død",
	"Ved %s sendes %s nu for dig.",
	"Ingen automatisk kommando ved %s længere.",
	"Dine automatiske kommandoer er slettet.",
	"Automatiske kommandoer: rundestart %s, rundeslut %s, død %s.",
	"Brug gamewe_auto start|end|kill <kommando> eller gamewe_auto clear. Kommandoer: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s kan ikke være en automatisk kommando.",
	"Sprog: %s.",
	"Brug gamewe_lang en, de, ru, es, fr, pt, nl, dk eller pl.",
	"Skjul mig på ranglisten",
	"Vis mig på ranglisten",
	"Skjul eller vis mig på ranglisten",
	"Skjult fra ranglisten (kun en admin kan ændre det)"
}

stock const gamewe_msg_pl[GAMEWE_MESSAGES][] = {
	"wł.",
	"wył.",
	"Zdarzenia w konsoli: %s",
	"Czat globalny: %s",
	"Użyj %s 1 (wł.) lub %s 0 (wył.).",
	"Zdarzenia w konsoli włączone: punkty za zabójstwa i akcje pojawiają się w konsoli.",
	"Zdarzenia w konsoli wyłączone.",
	"Zdarzenia czatu globalnego włączone.",
	"Zdarzenia czatu globalnego wyłączone.",
	"początek rundy",
	"koniec rundy",
	"twoja śmierć",
	"%s: teraz za ciebie wysyłane jest %s.",
	"%s: brak automatycznej komendy.",
	"Twoje automatyczne komendy zostały usunięte.",
	"Automatyczne komendy: początek rundy %s, koniec rundy %s, śmierć %s.",
	"Użyj gamewe_auto start|end|kill <komenda> lub gamewe_auto clear. Komendy: rank, kpd, session, next, top5, top10, top20, statsme",
	"%s nie może być automatyczną komendą.",
	"Język: %s.",
	"Użyj gamewe_lang en, de, ru, es, fr, pt, nl, dk lub pl.",
	"Ukryj mnie w rankingu",
	"Pokaż mnie w rankingu",
	"Ukryj lub pokaż mnie w rankingu",
	"Ukryty w rankingu (tylko admin może to zmienić)"
}

// index of a gamewe_lang code (or AMX Mod X language code), -1 if unknown
stock gamewe_lang_index(const code[])
{
	if (code[0] == EOS) {
		return GAMEWE_LANG_UNSET
	}
	for (new i = 0; i < GAMEWE_LANGUAGES; i++) {
		if (equali(code, gamewe_lang_codes[i])) {
			return i
		}
	}
	if (equali(code, "da")) {
		return 7
	}
	if (equali(code, "bp")) {
		return 5
	}
	return GAMEWE_LANG_UNSET
}

// the player's language: gamewe_lang, else the game's, else English (0)
stock gamewe_player_lang(id)
{
	if (id < 1 || id > MaxClients) {
		return 0
	}
	if (g_gamewe_lang[id] >= 0) {
		return g_gamewe_lang[id]
	}
	new info[8]
	get_user_info(id, "lang", info, charsmax(info))
	new index = gamewe_lang_index(info)
	return (index >= 0) ? index : 0
}

// message msg (GAMEWE_MSG_*) in the player's language
stock gamewe_msg(id, msg, out[], len)
{
	if (msg < 0 || msg >= GAMEWE_MESSAGES) {
		out[0] = EOS
		return
	}
	switch (gamewe_player_lang(id)) {
		case 1: copy(out, len, gamewe_msg_de[msg])
		case 2: copy(out, len, gamewe_msg_ru[msg])
		case 3: copy(out, len, gamewe_msg_es[msg])
		case 4: copy(out, len, gamewe_msg_fr[msg])
		case 5: copy(out, len, gamewe_msg_pt[msg])
		case 6: copy(out, len, gamewe_msg_nl[msg])
		case 7: copy(out, len, gamewe_msg_dk[msg])
		case 8: copy(out, len, gamewe_msg_pl[msg])
		default: copy(out, len, gamewe_msg_en[msg])
	}
}
