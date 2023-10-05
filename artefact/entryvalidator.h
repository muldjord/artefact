/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            entryvalidator.h
 *
 *  Mon Apr  2 11:14:41 CEST 2007
 *  Copyright 2007 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Artefact.
 *
 *  Artefact is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Artefact is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Artefact; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#ifndef __ARTEFACT_ENTRYVALIDATOR_H__
#define __ARTEFACT_ENTRYVALIDATOR_H__

#include "entry.h"

/**
 * This class is used to validate the contents of an Entry and subsequently
 * erase it or move it to a permanent storage.
 */
class EntryValidator {
public:
  /**
   * Constructor.\n
   * Connect this validator to an Entry.
   * @param entry A pointer to the entry on which to work.
   */
  EntryValidator(Pentominos::Entry &entry);

  /**
   * Validate the entry.
   * @return A boolean value of true if the Entry is valid, false if not.
   */
  bool validate();

  /**
   * Erase the entry from both memory and disk.
   */
  void erase();

  /**
   * Save the Entry by moving the temporary file(s) to a permanent residence.\n
   * NOTE: After this call, the entry should be saved in the database - 
   * <b>NOT</b> before!
   */
  void fixate();

private:
  Pentominos::Entry &entry;
};

#endif/*__ARTEFACT_ENTRYVALIDATOR_H__*/
