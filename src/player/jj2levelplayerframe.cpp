
/**
 *
 * @file jj2levelplayerframe.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 29th June 2010: Created jj2levelplayerframe.cpp from parts of
 *                 levelplayerframe.cpp
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
 * Provides the once-per-frame functions of players in levels.
 *
 */


#include "jj2levelplayer.h"

#include "game/game.h"
#include "game/gamemode.h"
#include "io/controls.h"
#include "io/gfx/font.h"
#include "io/gfx/video.h"
#include "io/sound.h"
#include "jj2level/jj2event/jj2event.h"
#include "jj2level/jj2level.h"
#include "util.h"


void JJ2LevelPlayer::modify (JJ2Modifier* nextMod, unsigned int ticks) {

	switch (nextMod->type) {

		case 2: // Spikes

			if (jj2Level->checkMaskDown(x + PXO_MID, y + F1, false))
				hit(NULL, ticks);

			break;

		case 4: // Hook

			dx = 0;

			break;

		case 6: // H-pole

			if (!stopTime) {

				// Catch player
				if (nextMod != mod) {

					centreX();
					centreY();
					stopTime = ticks + 1000;

				}

			} else if (ticks > stopTime) {

				dx = (dx > 0) ? PXS_POLE: -PXS_POLE;
				stopTime = 0;

			}

			break;

		case 7: // V-pole

			if (!stopTime) {

				// Catch player
				if (nextMod != mod) {

					centreX();
					centreY();
					stopTime = ticks + 1000;

				}

			} else if (ticks > stopTime) {

				if (dy < 0) {

					throwY = y - TTOF(16);
					dy = -PYS_POLE;

				} else {

					dy = PYS_POLE;

				}

				stopTime = 0;

			}

			break;

		case 8: // Fly off

			floating = false;

			break;

		case 17: // End of level
		case 18: // End of level

			if (!energy) return;

			if (!game->getMode()->endOfLevel(player, FTOT(x + PXO_MID), FTOT(y + PYO_MID))) return;

			break;

		case 206: // Sucker tube

			dx = (nextMod->properties & 0x40) ? -((nextMod->properties << 15) & 0x1F8000): ((nextMod->properties << 15) & 0x1F8000);
			dy = (nextMod->properties & 0x2000) ? -((nextMod->properties << 8) & 0x1F8000): ((nextMod->properties << 8) & 0x1F8000);

			if (dx) centreY();
			if (dy) centreX();

			break;

		case 207: // Text

			break;

		case 208: // Water level

			jj2Level->setWaterLevel(nextMod->properties & 0xFF, nextMod->properties & 0x100);

			break;

		case 230: // Warp

			if (!stopTime) {

				// Catch player
				if (coins >= ((nextMod->properties >> 8) & 255)) stopTime = ticks + 1000;

			} else if (ticks > stopTime) {

				coins -= (nextMod->properties >> 8) & 255;
				jj2Level->warp(this, nextMod->properties & 255);

				stopTime = 0;

			}

			break;

		default:

			stopTime = 0;

			break;

	}

	mod = nextMod;

	return;

}


/**
 * Respond to controls, unless the player has been killed
 */
void JJ2LevelPlayer::control (unsigned int ticks, int msps) {

	JJ2Modifier* nextMod;
	bool drop, platform;


	// If the player has been killed, do not move
	if (!energy) {

		dx = 0;
		dy = 0;

		animType = PA_DIE;

		return;

	}


	// Get overlapping modifier
	nextMod = jj2Level->getModifier(FTOT(x + PXO_MID), FTOT(y + PYO_MID));


	if (stopTime) {

		// Can't control player, so just apply modifier

		modify(nextMod, ticks);

		return;

	}


	if (player->pcontrols[C_RIGHT]) {

		// Walk/run right

		if (dx < 0) dx += PXA_REVERSE * msps;
		else if (dx < PXS_WALK) dx += PXA_WALK * msps;
		else if (dx < PXS_RUN) dx += PXA_RUN * msps;

		facing = true;

	} else if (player->pcontrols[C_LEFT]) {

		// Walk/run left

		if (dx > 0) dx -= PXA_REVERSE * msps;
		else if (dx > -PXS_WALK) dx -= PXA_WALK * msps;
		else if (dx > -PXS_RUN) dx -= PXA_RUN * msps;

		facing = false;

	} else if ((nextMod->type >> 1) != 3) {

		// Slow down

		if (dx > 0) {

			if (dx < PXA_STOP * msps) dx = 0;
			else dx -= PXA_STOP * msps;

		}

		if (dx < 0) {

			if (dx > -PXA_STOP * msps) dx = 0;
			else dx += PXA_STOP * msps;

		}

	}


	drop = player->pcontrols[C_DOWN];

	// Check for platform event, bridge or level mask below player
	platform = (event == LPE_PLATFORM) ||
		jj2Level->checkMaskDown(x + PXO_ML, y + 1, drop) ||
		jj2Level->checkMaskDown(x + PXO_MID, y + 1, drop) ||
		jj2Level->checkMaskDown(x + PXO_MR, y + 1, drop) ||
		((dx > 0) && jj2Level->checkMaskDown(x + PXO_ML, y + F8, drop)) ||
		((dx < 0) && jj2Level->checkMaskDown(x + PXO_MR, y + F8, drop));

	if (floating) {

		if (player->pcontrols[C_UP]) {

			// Fly upwards

			if (dy > 0) dy -= PXA_REVERSE * msps;
			else if (dy > -PXS_WALK) dy -= PXA_WALK * msps;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * msps;

		} else if (player->pcontrols[C_DOWN]) {

			// Fly downwards

			if (dy < 0) dy += PXA_REVERSE * msps;
			else if (dy < PXS_WALK) dy += PXA_WALK * msps;
			else if (dy < PXS_RUN) dy += PXA_RUN * msps;

		} else if ((nextMod->type >> 1) != 3) {

			// Slow down

			if (dy > 0) {

				if (dy < PXA_STOP * msps) dy = 0;
				else dy -= PXA_STOP * msps;

			}

			if (dy < 0) {

				if (dy > -PXA_STOP * msps) dy = 0;
				else dy += PXA_STOP * msps;

			}

		}

		if (event != LPE_NONE) {

			if (event == LPE_SPRING) dy = PYS_SPRING;
			else if (event == LPE_FLOAT) dy = PYS_JUMP;

		}

	} else if (y + PYO_MID > jj2Level->getWaterLevel()) {

		if (player->pcontrols[C_SWIM]) {

			// Swim upwards

			if (dy > 0) dy -= PXA_REVERSE * msps;
			else if (dy > -PXS_WALK) dy -= PXA_WALK * msps;
			else if (dy > -PXS_RUN) dy -= PXA_RUN * msps;

			// Prepare to jump upon leaving the water

			if (!jj2Level->checkMaskUp(x + PXO_MID, y - F36)) {

				throwY = y - jumpHeight;

				if (dx < 0) throwY += dx >> 4;
				else if (dx > 0) throwY -= dx >> 4;

				event = LPE_NONE;

			}

		} else if (player->pcontrols[C_DOWN]) {

			// Swim downwards

			if (dy < 0) dy += PXA_REVERSE * msps;
			else if (dy < PXS_WALK) dy += PXA_WALK * msps;
			else if (dy < PXS_RUN) dy += PXA_RUN * msps;

		} else {

			// Sink

			dy += PYA_SINK * msps;
			if (dy > PYS_SINK) dy = PYS_SINK;

		}

	} else {

		if (platform && player->pcontrols[C_JUMP] &&
			!jj2Level->checkMaskUp(x + PXO_MID, y - F36)) {

			// Jump

			throwY = y - jumpHeight;

			// Increase jump height if walking/running
			if (dx < 0) throwY += dx >> 3;
			else if (dx > 0) throwY -= dx >> 3;

			event = LPE_NONE;

			playSound(S_JUMPA);

		}

		// Stop jumping
		if (!player->pcontrols[C_JUMP] && (event != LPE_SPRING) && (event != LPE_FLOAT))
			throwY = TTOF(256);

		if (y >= throwY) {

			// If jumping, rise
			dy = (throwY - y - F64) * 4;

			// Avoid jumping too fast, unless caused by an event
			if ((event == LPE_NONE) && (dy < PYS_JUMP)) dy = PYS_JUMP;

		} else if (!platform) {

			// Fall under gravity
			dy += PYA_GRAVITY * msps;
			if (dy > PYS_FALL) dy = PYS_FALL;

		}

		// Don't descend through platforms
		if ((dy > 0) && (event == LPE_PLATFORM)) dy = 0;

		if (platform && !lookTime) {

			// If requested, look up or down
			if (player->pcontrols[C_UP]) lookTime = -ticks;
			else if (player->pcontrols[C_DOWN]) lookTime = ticks;

		}

		// Stop looking if there is no platform or the control has been released
		if (!platform ||
			(!player->pcontrols[C_UP] && (lookTime < 0)) ||
			(!player->pcontrols[C_DOWN] && (lookTime > 0))) lookTime = 0;

	}

	// If there is an obstacle above and the player is not floating up, stop
	// rising
	if (jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - F1) && (throwY < y) && (event != LPE_FLOAT)) {

		throwY = TTOF(256);
		if (dy < 0) dy = 0;

		if (event != LPE_PLATFORM) event = LPE_NONE;

	}

	// If jump completed, stop rising
	if (y <= throwY) {

		throwY = TTOF(256);

		if (event != LPE_PLATFORM) event = LPE_NONE;

	}


	// Apply modifier
	modify(nextMod, ticks);


	// Limit speed

	if (dx < -PXS_LIMIT) dx = -PXS_LIMIT;
	else if (dx > PXS_LIMIT) dx = PXS_LIMIT;

	if (dy < -PYS_LIMIT) dy = -PYS_LIMIT;
	else if (dy > PYS_LIMIT) dy = PYS_LIMIT;


	// Handle firing
	if (player->pcontrols[C_FIRE]) {

		if (ticks > fireTime) {

			// Make sure bullet position is taken from correct animation
			if (platform) animType = PA_STANDSHOOT;

			/// @todo Create new bullet when firing

			// Set when the next bullet can be fired
			if (player->fireSpeed) fireTime = ticks + (1000 / player->fireSpeed);
			else fireTime = 0x7FFFFFFF;

			// Remove the bullet from the arsenal
			if (player->ammoType != -1) player->ammo[player->ammoType]--;

			/* If the current ammo type has been exhausted, use the previous
			non-exhausted ammo type */
			while ((player->ammoType > -1) && !player->ammo[player->ammoType]) player->ammoType--;

		}

	} else fireTime = 0;


	// Check for a change in ammo
	if (player->pcontrols[C_CHANGE]) {

		if (player == localPlayer) controls.release(C_CHANGE);

		player->ammoType = ((player->ammoType + 2) % 5) - 1;

		// If there is no ammo of this type, go to the next type that has ammo
		while ((player->ammoType > -1) && !player->ammo[player->ammoType])
			player->ammoType = ((player->ammoType + 2) % 5) - 1;

	}


	// Deal with the bird

	/*if (bird) {

		if (bird->step(ticks, msps)) {

			delete bird;
			bird = NULL;

		}

	}*/


	// Choose animation

	if ((reaction == JJ2PR_HURT) && (reactionTime - ticks > PRT_HURT - PRT_HURTANIM))
		animType = PA_HURT1;

	else if (y + PYO_MID > jj2Level->getWaterLevel())
		animType = PA_SWIM;

	else if (floating) animType = PA_BOARD;

	else if (dy < 0) {

		if (event == LPE_SPRING) animType = PA_FLOAT1;
		else animType = PA_JUMP2;

	} else if (platform) {

		if (dx) {

			if (dx <= -PXS_RUN) animType = PA_RUN;
			else if (dx >= PXS_RUN) animType = PA_RUN;
			else if ((dx < 0) && facing) animType = PA_STOP1;
			else if ((dx > 0) && !facing) animType = PA_STOP1;
			else animType = PA_WALK2;

		} else if (!jj2Level->checkMaskDown(x + PXO_ML, y + F12, drop) &&
			!jj2Level->checkMaskDown(x + PXO_L, y + F2, drop) &&
			(event != LPE_PLATFORM))
			animType = PA_EDGE;

		else if (!jj2Level->checkMaskDown(x + PXO_MR, y + F12, drop) &&
			!jj2Level->checkMaskDown(x + PXO_R, y + F2, drop) &&
			(event != LPE_PLATFORM))
			animType = PA_EDGE;

		else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime))
			animType = PA_LOOKUP1;

		else if (lookTime > 0) {

			if ((int)ticks < 1000 + lookTime) animType = PA_CROUCHED;
			else animType = PA_CROUCH1;

		}

		else if (player->pcontrols[C_FIRE]) animType = PA_STANDSHOOT;

		else animType = PA_STAND;

	} else animType = PA_FALL;


	return;

}


void JJ2LevelPlayer::move (unsigned int ticks, int msps) {

	fixed pdx, pdy;
	int count;
	bool drop;


	if (stopTime) return;


	// Apply as much of the trajectory as possible, without going into the
	// scenery

	if (fastFeetTime > ticks) {

		pdx = (dx * msps * 3) >> 11;
		pdy = (dy * msps * 3) >> 11;

	} else {

		pdx = (dx * msps) >> 10;
		pdy = (dy * msps) >> 10;

	}


	// First for the vertical component of the trajectory

	drop = player->pcontrols[C_DOWN];

	if (pdy < 0) {

		// Moving up

		count = (-pdy) >> 10;

		while (count > 0) {

			if (jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - F1)) {

				y &= ~1023;
				dy = 0;

				break;

			}

			y -= F1;
			count--;

		}

		pdy = (-pdy) & 1023;

		if (!jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - pdy))
			y -= pdy;
		else {

			y &= ~1023;
			dy = 0;

		}

	} else if (pdy > 0) {

		// Moving down

		count = pdy >> 10;

		while (count > 0) {

			if (jj2Level->checkMaskDown(x + PXO_ML, y + F1, drop) ||
				jj2Level->checkMaskDown(x + PXO_MID, y + F1, drop) ||
				jj2Level->checkMaskDown(x + PXO_MR, y + F1, drop)) {

				y |= 1023;
				dy = 0;

				break;

			}

			y += F1;
			count--;

		}

		pdy &= 1023;

		if (!(jj2Level->checkMaskDown(x + PXO_ML, y + pdy, drop) ||
			jj2Level->checkMaskDown(x + PXO_MID, y + pdy, drop) ||
			jj2Level->checkMaskDown(x + PXO_MR, y + pdy, drop)))
			y += pdy;
		else {

			y |= 1023;
			dy = 0;

		}

	}



	// Then for the horizontal component of the trajectory

	if (pdx < 0) {

		// Moving left

		count = (-pdx) >> 10;

		while (count > 0) {

			// If there is an obstacle, stop
			if (jj2Level->checkMaskUp(x + PXO_L - F1, y + PYO_MID)) {

				x &= ~1023;

				break;

			}

			x -= F1;
			count--;

			// If on an uphill slope, push the player upwards
			if (jj2Level->checkMaskUp(x + PXO_ML, y) &&
				!jj2Level->checkMaskUp(x + PXO_ML, y - F1)) y -= F1;

		}

		pdx = (-pdx) & 1023;

		if (!jj2Level->checkMaskUp(x + PXO_L - pdx, y + PYO_MID)) x -= pdx;
		else x &= ~1023;

		// If on an uphill slope, push the player upwards
		while (jj2Level->checkMaskUp(x + PXO_ML, y) &&
			!jj2Level->checkMaskUp(x + PXO_ML, y - F1)) y -= F1;

	} else if (pdx > 0) {

		// Moving right

		count = pdx >> 10;

		while (count > 0) {

			// If there is an obstacle, stop
			if (jj2Level->checkMaskUp(x + PXO_R + F1, y + PYO_MID)) {

				x |= 1023;

				break;

			}

			x += F1;
			count--;

			// If on an uphill slope, push the player upwards
			if (jj2Level->checkMaskUp(x + PXO_MR, y) &&
				!jj2Level->checkMaskUp(x + PXO_MR, y - F1)) y -= F1;

		}

		pdx &= 1023;

		if (!jj2Level->checkMaskUp(x + PXO_R + pdx, y + PYO_MID)) x += pdx;
		else x |= 1023;

		// If on an uphill slope, push the player upwards
		while (jj2Level->checkMaskUp(x + PXO_MR, y) &&
			!jj2Level->checkMaskUp(x + PXO_MR, y - F1)) y -= F1;

	}


	// If using a float up event and have hit a ceiling, ignore event
	if ((event == LPE_FLOAT) && jj2Level->checkMaskUp(x + PXO_MID, y + PYO_TOP - F1)) {

		throwY = TTOF(256);
		event = LPE_NONE;

	}


	if (jj2Level->getStage() == LS_END) return;


	return;

}


/**
 * Calculate viewport
 */
void JJ2LevelPlayer::view (unsigned int ticks, int mspf) {

	int oldViewX, oldViewY, speed;

	// Record old viewport position for applying lag
	oldViewX = viewX;
	oldViewY = viewY;


	// Find new position

	viewX = x + F8 - (canvasW << 9);
	viewY = y - F24 - (canvasH << 9);

	if ((lookTime > 0) && ((int)ticks > 1000 + lookTime)) {

		// Look down
		if ((int)ticks < 2000 + lookTime) viewY += 64 * (ticks - (1000 + lookTime));
		else viewY += F64;

	} else if ((lookTime < 0) && ((int)ticks > 1000 - lookTime)) {

		// Look up
		if ((int)ticks < 2000 - lookTime) viewY -= 64 * (ticks - (1000 - lookTime));
		else viewY -= F64;

	}


	// Apply lag proportional to player "speed"
	speed = ((dx >= 0? dx: -dx) + (dy >= 0? dy: -dy)) >> 14;

	if (speed && (mspf < speed)) {

		viewX = ((oldViewX * (speed - mspf)) + (viewX * mspf)) / speed;
		viewY = ((oldViewY * (speed - mspf)) + (viewY * mspf)) / speed;

	}


	return;

}

void JJ2LevelPlayer::draw (unsigned int ticks, int change) {

	Anim *an;
	int frame;
	fixed drawX, drawY;

	// The current frame for animations
	if (reaction == JJ2PR_KILLED) frame = (ticks + PRT_KILLED - reactionTime) / 75;
	else frame = ticks / 75;


	// Get position

	drawX = getDrawX(stopTime? 0: change);
	drawY = getDrawY(stopTime? 0: change);


	// Choose sprite

	an = getAnim();
	an->setFrame(frame, reaction != JJ2PR_KILLED);


	// Show the player

	// Use player colour
	an->setPalette(palette, 16, 32);

	// Flash on and off if hurt
	if ((reaction != JJ2PR_HURT) || ((ticks / 30) & 2)) {

		// Draw "motion blur"
		if (fastFeetTime > ticks) an->draw(drawX + F16 - (dx >> 6), drawY + F16);

		// Draw player
		an->draw(drawX + F16, drawY + F16);

	}


	// Uncomment the following to see the area of the player
	/*drawRect(FTOI(drawX + PXO_L - viewX),
		FTOI(drawY + PYO_TOP - viewY),
		FTOI(PXO_R - PXO_L),
		FTOI(-PYO_TOP), 89);
	drawRect(FTOI(drawX + PXO_ML - viewX),
		FTOI(drawY + PYO_TOP - viewY),
		FTOI(PXO_MR - PXO_ML),
		FTOI(-PYO_TOP), 88);*/


	if (reaction == JJ2PR_INVINCIBLE) {

		/// @todo Show invincibility effect

	}

	switch (shield) {

		case JJ2S_NONE:

			// Do nothing

			break;

		case JJ2S_FLAME:

			/// @todo Show shield effect

			break;

		case JJ2S_BUBBLE:

			/// @todo Show shield effect

			break;

		case JJ2S_PLASMA:

			/// @todo Show shield effect

			break;

		case JJ2S_LASER:

			/// @todo Show shield effect

			break;

	}


	// Show the bird
	//if (bird) bird->draw(ticks, change);


	// Show the player's name
	if (nPlayers > 1)
		panelBigFont->showString(player->name,
			FTOI(drawX + PXO_MID) - (panelBigFont->getStringWidth(player->name) >> 1),
			FTOI(drawY - F32 - F16));

	//panelBigFont->showNumber(mod->properties, FTOI(drawX) + 24, FTOI(drawY) + 12);

	return;

}


