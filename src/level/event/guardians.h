
/**
 *
 * @file guardians.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 2nd March 2010: Created guardians.h from parts of event.h
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


#ifndef _GUARDIANS_H
#define _GUARDIANS_H


#include "event.h"


// Class

/// Guardian event base class
class Guardian : public Event {

	protected:
		int stage;

		Guardian (unsigned char gX, unsigned char gY);

};

/// Episode B guardian
class DeckGuardian : public Guardian {

	public:
		DeckGuardian (unsigned char gX, unsigned char gY);

		bool   overlap (fixed left, fixed top, fixed width, fixed height);
		Event* step    (unsigned int ticks, int msps);
		void   draw    (unsigned int ticks, int change);

};

/// Episode 1 guardian
class MedGuardian : public Guardian {

	private:
		unsigned char direction;
		bool shoot;

	public:
		MedGuardian (unsigned char gX, unsigned char gY);

		//bool   overlap (fixed left, fixed top, fixed width, fixed height);
		Event* step    (unsigned int ticks, int msps);
		void   draw    (unsigned int ticks, int change);

};


#endif

