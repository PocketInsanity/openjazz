
/**
 *
 * @file jj2levelplayer.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 29th June 2010: Created jj2levelplayer.cpp from parts of levelplayer.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with the creation and destruction of players in levels, and their
 * interactions with other level objects.
 *
 */


#include "jj2levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/sound.h"
#include "jj2level/jj2event/jj2event.h"
#include "jj2level/jj2level.h"

#include <string.h>


JJ2LevelPlayer::JJ2LevelPlayer (Player* parent, Anim** newAnims, unsigned char startX, unsigned char startY, bool hasBird) {

	int offsets[14] = {JJ2PCO_GREY, JJ2PCO_SGREEN, JJ2PCO_BLUE, JJ2PCO_RED,
		JJ2PCO_LGREEN, JJ2PCO_LEVEL1, JJ2PCO_YELLOW, JJ2PCO_LEVEL2,
		JJ2PCO_ORANGE, JJ2PCO_LEVEL3, JJ2PCO_LEVEL4, JJ2PCO_SANIM, JJ2PCO_LANIM,
		JJ2PCO_LEVEL5};
	int lengths[14] = {JJ2PCL_GREY, JJ2PCL_SGREEN, JJ2PCL_BLUE, JJ2PCL_RED,
		JJ2PCL_LGREEN, JJ2PCL_LEVEL1, JJ2PCL_YELLOW, JJ2PCL_LEVEL2,
		JJ2PCL_ORANGE, JJ2PCL_LEVEL3, JJ2PCL_LEVEL4, JJ2PCL_SANIM, JJ2PCL_LANIM,
		JJ2PCL_LEVEL5};
	int count, start, length;


	player = parent;

	anims = newAnims[0];
	flippedAnims = newAnims[1];

	bird = hasBird;

	shield = JJ2S_NONE;

	reset(startX, startY);


	// Create the player's palette

	for (count = 0; count < 256; count++)
		palette[count].r = palette[count].g = palette[count].b = count;


	// Fur colours

	start = offsets[player->cols[0]];
	length = lengths[player->cols[0]];

	for (count = 0; count < 16; count++)
		palette[count + 16].r = palette[count + 16].g = palette[count + 16].b =
			(count * length / 8) + start;


	// Bandana colours

	start = offsets[player->cols[1]];
	length = lengths[player->cols[1]];

	for (count = 0; count < 16; count++)
		palette[count + 24].r = palette[count + 24].g = palette[count + 24].b =
 			(count * length / 8) + start;


	// Gun colours

	start = offsets[player->cols[2]];
	length = lengths[player->cols[2]];

	for (count = 0; count < 9; count++)
		palette[count + 32].r = palette[count + 32].g = palette[count + 32].b =
			(count * length / 8) + start;


	// Wristband colours

	start = offsets[player->cols[3]];
	length = lengths[player->cols[3]];

	for (count = 0; count < 8; count++)
		palette[count + 40].r = palette[count + 40].g = palette[count + 40].b =
			(count * length / 8) + start;


	return;

}


JJ2LevelPlayer::~JJ2LevelPlayer () {

	return;

}


void JJ2LevelPlayer::reset (unsigned char startX, unsigned char startY) {

	mod = NULL;
	energy = 5;
	floating = false;
	facing = true;
	animType = PA_STAND;
	event = LPE_NONE;
	reaction = JJ2PR_NONE;
	reactionTime = 0;
	jumpHeight = ITOF(92);
	throwY = TTOF(256);
	fastFeetTime = 0;
	stopTime = 0;
	dx = 0;
	dy = 0;
	x = TTOF(startX);
	y = TTOF(startY);
	gems[0] = gems[1] = gems[2] = gems[3] = 0;
	coins = 0;

	return;

}


void JJ2LevelPlayer::addGem (int colour) {

	gems[colour]++;

	return;

}


void JJ2LevelPlayer::centreX () {

	x = ((x + PXO_MID) & ~32767) + F16 - PXO_MID;

	return;

}


void JJ2LevelPlayer::centreY () {

	y = ((y + PYO_MID) & ~32767) + F16 - PYO_MID;

	return;

}


Anim* JJ2LevelPlayer::getAnim () {

	return (facing? anims: flippedAnims) + animType;

}


int JJ2LevelPlayer::getEnergy () {

	return energy;

}


bool JJ2LevelPlayer::getFacing () {

	return facing;

}


int JJ2LevelPlayer::getGems (int colour) {

	return gems[colour];

}


bool JJ2LevelPlayer::hasBird () {

	return bird;

}


bool JJ2LevelPlayer::hit (Player *source, unsigned int ticks) {

	// Invulnerable if reacting to e.g. having been hit
	if (reaction != JJ2PR_NONE) return false;

	// Hits from the same team have no effect
	if (source && (source->getTeam() == player->team)) return false;


	if (game->getMode()->hit(source, player)) {

		energy--;

		//if (bird) bird->hit();

		playSound(S_OW);

	}

	if (energy) {

		reaction = JJ2PR_HURT;
		reactionTime = ticks + PRT_HURT;

		if (dx < 0) {

			dx = PXS_RUN;
			dy = PYS_JUMP;

		} else {

			dx = -PXS_RUN;
			dy = PYS_JUMP;

		}

	} else {

		kill(source, ticks);

	}

	return true;

}


void JJ2LevelPlayer::kill (Player *source, unsigned int ticks) {

	if (reaction != JJ2PR_NONE) return;

	if (game->getMode()->kill(source, player)) {

		energy = 0;
		player->lives--;

		reaction = JJ2PR_KILLED;
		reactionTime = ticks + PRT_KILLED;

	}

	return;

}


bool JJ2LevelPlayer::overlap (fixed left, fixed top, fixed width, fixed height) {

	return (x + PXO_R >= left) && (x + PXO_L < left + width) &&
		(y >= top) && (y + PYO_TOP < top + height);

}


JJ2PlayerReaction JJ2LevelPlayer::reacted (unsigned int ticks) {

	JJ2PlayerReaction oldReaction;

	if ((reaction != JJ2PR_NONE) && (reactionTime < ticks)) {

		oldReaction = reaction;
		reaction = JJ2PR_NONE;

		return oldReaction;

	}

	return JJ2PR_NONE;

}


void JJ2LevelPlayer::setPosition (fixed newX, fixed newY) {

	x = newX;
	y = newY;

	return;

}


void JJ2LevelPlayer::setSpeed (fixed newDx, fixed newDy) {

	dx = newDx;
	if (newDy) dy = newDy;

	return;

}


bool JJ2LevelPlayer::touchEvent (JJ2Event* touched, unsigned int ticks, int msps) {

	unsigned char type;
	bool fullPickup = false;

	type = touched->getType();

	switch (type) {

		case 34: // Ammo

			player->addAmmo(2, 3);
			player->addScore(100);

			return true;

		case 36: // Ammo

			player->addAmmo(1, 3);
			player->addScore(100);

			return true;

		case 37: // Ammo

			player->addAmmo(0, 3);
			player->addScore(100);

			return true;

		case 38: // TNT

			player->addAmmo(3, 3);
			player->addScore(100);

			return true;

		case 44:

			coins++;
			player->addScore(500);

			return true;

		case 45:

			coins += 5;
			player->addScore(1000);

			return true;

		case 59: // Board

			floating = true;

			return true;

		case 60: // Frozen green spring

			throwY = y - TTOF(14);
			dx = 0;
			event = LPE_SPRING;

			break;

		case 61: // Rapid fire

			player->fireSpeed++;

			return true;

		case 62: // Spring crate

			throwY = y - TTOF(18);
			dx = 0;
			event = LPE_SPRING;

			break;

		case 63: // Red gem

			gems[0]++;
			player->addScore(100);

			return true;

		case 64: // Green gem

			gems[1]++;
			player->addScore(100);

			return true;

		case 65: // Blue gem

			gems[2]++;
			player->addScore(100);

			return true;

		case 66: // Purple gem

			gems[3]++;
			player->addScore(100);

			return true;

		case 72: // Carrot

			if ((energy < 5) || fullPickup) {

				if (energy < 5) energy++;

				player->addScore(200);

				return true;

			}

			break;

		case 73: // Full carrot

			if ((energy < 5) || fullPickup) {

				energy = 5;

				return true;

			}

			break;

		case 80: // 1-up

			player->addLife();

			return true;

		case 83: // Checkpoint

			game->setCheckpoint(FTOT(x + PXO_MID), FTOT(y + PYO_MID));

			break;

		case 85: // Red spring

			throwY = y - TTOF(8);
			dx = 0;
			event = LPE_SPRING;

			break;

		case 86: // Green spring

			throwY = y - TTOF(14);
			dx = 0;
			event = LPE_SPRING;

			break;

		case 87: // Blue spring

			throwY = y - TTOF(18);
			dx = 0;
			event = LPE_SPRING;

			break;

		case 88: //Invincibility

			reaction = JJ2PR_INVINCIBLE;
			reactionTime = ticks + PRT_INVINCIBLE;

			return true;

		case 91: // Horizontal red spring

			if (true) throwX = x + TTOF(7);
			else throwX = x - TTOF(7);

			dy = 0;

			break;

		case 92: // Horizontal green spring

			if (true) throwX = x + TTOF(14);
			else throwX = x - TTOF(14);

			dy = 0;

			break;

		case 93: // Horizontal blue spring

			if (true) throwX = x + TTOF(18);
			else throwX = x - TTOF(18);

			dy = 0;

			break;

		case 96: // Helicarrot

			floating = true;

			return true;

		case 192: // Gem ring

			gems[0] += 8;

			return true;

		default:

			if (((type >= 141) && (type <= 147)) || ((type >= 154) && (type <= 182))) {

				// Food
				player->addScore(50);

				return true;

			}

			break;

	}

	return false;

}


/**
 * Copy data to be sent to clients/server
 */
void JJ2LevelPlayer::send (unsigned char *buffer) {

	buffer[9] = bird? 1: 0;
	buffer[23] = energy;
	buffer[25] = shield;
	buffer[26] = floating;
	buffer[27] = getFacing();
	buffer[29] = jumpHeight >> 24;
	buffer[30] = (jumpHeight >> 16) & 255;
	buffer[31] = (jumpHeight >> 8) & 255;
	buffer[32] = jumpHeight & 255;
	buffer[33] = throwY >> 24;
	buffer[34] = (throwY >> 16) & 255;
	buffer[35] = (throwY >> 8) & 255;
	buffer[36] = throwY & 255;
	buffer[37] = x >> 24;
	buffer[38] = (x >> 16) & 255;
	buffer[39] = (x >> 8) & 255;
	buffer[40] = x & 255;
	buffer[41] = y >> 24;
	buffer[42] = (y >> 16) & 255;
	buffer[43] = (y >> 8) & 255;
	buffer[44] = y & 255;

	return;

}


/**
 * Interpret data received from client/server
 */
void JJ2LevelPlayer::receive (unsigned char *buffer) {

	switch (buffer[1]) {

		case MT_P_ANIMS:

			anims = jj2Level->getAnim(buffer[3], 0, false);
			flippedAnims = jj2Level->getAnim(buffer[3], 0, true);

			break;

		case MT_P_TEMP:

			if ((buffer[9] & 1) && !bird) bird = true;

			if (!(buffer[9] & 1) && bird) {

				bird = false;

			}

			energy = buffer[23];
			shield = (JJ2Shield)buffer[25];
			floating = buffer[26];
			facing = buffer[27];
			jumpHeight = (buffer[29] << 24) + (buffer[30] << 16) + (buffer[31] << 8) + buffer[32];
			throwY = (buffer[33] << 24) + (buffer[34] << 16) + (buffer[35] << 8) + buffer[36];
			x = (buffer[37] << 24) + (buffer[38] << 16) + (buffer[39] << 8) + buffer[40];
			y = (buffer[41] << 24) + (buffer[42] << 16) + (buffer[43] << 8) + buffer[44];

			break;

	}

	return;

}

