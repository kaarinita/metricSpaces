// make sure this include goes before:
#include "FTed.hpp"
// this one:
extern "C" {
#include "../../obj.h"v
}
// otherwise some weird compilation errors will appear.
/*
 * Copyright (C) 2009 Arnoldo Jose Muller Molina
 *  This program is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version. This program is distributed
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received
 * a copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
static FTed DB;

extern "C"{
/**
 * Calculate the mtd distance between objects obj1 and obj2
 */
Tdist distanceInter (Obj obj1, Obj obj2){
	return DB.ted(obj1,obj2);
}


/**
 * Parse an object from the string.
 */
Obj parseobj (char *s){
	DB.setQuery(string(s));
	return NewObj; 
}

/*
 * print the given object (dummy)
 */ 
void printobj (Obj obj) { 
	printf ("%i\n",obj);
}

/**
 * Open the database of trees.
 */
int openDB (char *name){
	DB.loadDB(string(name));
	return DB.getSize();
}

/**
 * Close the database.
 */
void closeDB (void){
	DB.clean();
}


}
