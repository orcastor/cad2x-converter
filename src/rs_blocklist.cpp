/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/

#include <set>
#include <iostream>
#include <QString>
#include <QRegExp>
#include "rs_debug.h"
#include "rs_blocklist.h"
#include "rs_block.h"

/**
 * Constructor.
 * 
 * @param owner true if this is the owner of the blocks added.
 *              If so, the blocks will be deleted when the block
 *              list is deleted.
 */
RS_BlockList::RS_BlockList(bool owner) {
    this->owner = owner;
    //blocks.setAutoDelete(owner);
	activeBlock = nullptr;
}


/**
 * Removes all blocks in the blocklist.
 */
void RS_BlockList::clear() {
    blocks.clear();
	activeBlock = nullptr;
}


/**
 * Activates the given block.
 * Listeners are notified.
 */
void RS_BlockList::activate(const QString& name) {
    RS_DEBUG->print("RS_BlockList::activateBlock");

    activate(find(name));
}

/**
 * Activates the given block.
 * Listeners are notified.
 */
void RS_BlockList::activate(RS_Block* block) {
    RS_DEBUG->print("RS_BlockList::activateBlock");
	activeBlock = block;
}


/**
 * Adds a block to the block list. If a block with the same name
 * exists already, the given block will be deleted if the blocklist
 * owns the blocks.
 *
 * @param notify true if you want listeners to be notified.
 *
 * @return false: block already existed and was deleted.
 */
bool RS_BlockList::add(RS_Block* block) {
    RS_DEBUG->print("RS_BlockList::add()");

	if (!block) {
        return false;
    }

    // check if block already exists:
    RS_Block* b = find(block->getName());
	if (!b) {
        blocks.append(block);
		return true;
    } else {
        if (owner) {
            delete block;
			block = nullptr;
        }
		return false;
    }

}



/**
 * Removes a block from the list.
 * Listeners are notified after the block was removed from 
 * the list but before it gets deleted.
 */
void RS_BlockList::remove(RS_Block* block) {
    RS_DEBUG->print("RS_BlockList::removeBlock()");

    // here the block is removed from the list but not deleted
    blocks.removeOne(block);

    // / *
    // activate an other block if necessary:
    if (activeBlock==block) {
    	//activate(blocks.first());
		activeBlock = nullptr;
	}
    // * /

    // now it's save to delete the block
    if (owner) {
        delete block;
    }
}



/**
 * Tries to rename the given block to 'name'. Block names are unique in the
 * block list.
 *
 * @retval true block was successfully renamed.
 * @retval false block couldn't be renamed.
 */
bool RS_BlockList::rename(RS_Block* block, const QString& name) {
	if (block) {
		if (!find(name)) {
			QString oldName = block->getName();
			block->setName(name);

			// when the renamed block is nested within other block, we need to rename its inserts as well
			for(RS_Block* b: blocks) {
				b->renameInserts(oldName, name);
			}

			return true;
		}
	}

	return false;
}

/**
 * @return Pointer to the block with the given name or
 * \p nullptr if no such block was found.
 */
RS_Block* RS_BlockList::find(const QString& name) {
	// Todo : reduce this from O(N) to O(log(N)) complexity based on sorted list or hash
	//DFS
	for(RS_Block* b: blocks) {
		if (b->getName()==name) {
			return b;
		}
	}
	return nullptr;
}

/**
 * Finds a new unique block name.
 *
 * @param suggestion Suggested name the new name will be based on.
 */
QString RS_BlockList::newName(const QString& suggestion) {
//	qDebug()<<"begin: suggestion: "<<suggestion;
	if(!find(suggestion))
		return suggestion;

	QString name=suggestion;
	QRegExp const rx(R"(-\d+$)");
	int index=name.lastIndexOf(rx);
	int i=-1;
	if(index>0){
		i=name.mid(index+1).toInt();
		name=name.mid(0, index);
	}
	for(RS_Block* b: blocks){
		index=b->getName().lastIndexOf(rx);
		if(index<0) continue;
		QString const part1= b->getName().mid(0, index);
		if(part1 != name) continue;
		i=std::max(b->getName().mid(index+1).toInt(),i);
	}
//	qDebug()<<QString("%1-%2").arg(name).arg(i+1);
	return QString("%1-%2").arg(name).arg(i+1);
}

/**
 * Switches on / off the given block. 
 * Listeners are notified.
 */
void RS_BlockList::toggle(const QString& name) {
    toggle(find(name));
}

/**
 * Switches on / off the given block. 
 * Listeners are notified.
 */
void RS_BlockList::toggle(RS_Block* block) {
	if (!block) {
        return;
    }

    block->toggle();
}

/**
 * Freezes or defreezes all blocks.
 *
 * @param freeze true: freeze, false: defreeze
 */
void RS_BlockList::freezeAll(bool freeze) {

    for (int l=0; l<count(); l++) {
        if (at(l)->isVisibleInBlockList()) {
            at(l)->freeze(freeze);
        }
    }
}

int RS_BlockList::count() const{
	return blocks.count();
}

/**
 * @return Block at given position or nullptr if i is out of range.
 */
RS_Block* RS_BlockList::at(int i) {
	return blocks.at(i);
}
RS_Block* RS_BlockList::at(int i) const{
	return blocks.at(i);
}
QList<RS_Block*>::iterator RS_BlockList::begin()
{
	return blocks.begin();
}

QList<RS_Block*>::iterator RS_BlockList::end()
{
	return blocks.end();
}

QList<RS_Block*>::const_iterator RS_BlockList::begin()const
{
	return blocks.begin();
}

QList<RS_Block*>::const_iterator RS_BlockList::end()const
{
	return blocks.end();
}

//! @return The active block of nullptr if no block is activated.
RS_Block* RS_BlockList::getActive() {
	return activeBlock;
}

