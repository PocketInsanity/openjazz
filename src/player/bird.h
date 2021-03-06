
/**
 *
 * @file bird.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 1st March 2009: Created bird.h from parts of events.h
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
 */


#ifndef _BIRD_H
#define _BIRD_H


#include "level/movable.h"
#include "OpenJazz.h"


// Constants

// Animations
#define BIRD_LEFTANIM  51
#define BIRD_RIGHTANIM 52

// Time interval
#define T_BIRD_FIRE 500


// Classes

class LevelPlayer;

/// JJ1 bird companion
class Bird : public Movable {

	private:
		LevelPlayer* player; ///< Player that rescued the bird
		bool         fleeing; ///< Flying away, player having been shot
		unsigned int fireTime; ///< Next time the bird will fire

	public:
		Bird  (LevelPlayer* player, unsigned char gX, unsigned char gY);
		~Bird ();

		LevelPlayer* getPlayer ();
		void         hit       ();

		bool         step      (unsigned int ticks, int msps);
		void         draw      (unsigned int ticks, int change);

};

#endif

