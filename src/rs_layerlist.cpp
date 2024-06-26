/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2015 A. Stebich (librecad@mail.lordofbikes.de)
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

#include<iostream>

#include "rs_debug.h"
#include "rs_layerlist.h"
#include "rs_layer.h"

/**
 * Default constructor.
 */
RS_LayerList::RS_LayerList() {
    activeLayer = nullptr;
}

/**
 * Removes all layers in the layerlist.
 */
void RS_LayerList::clear() {
    layers.clear();
}

QList<RS_Layer*>::iterator RS_LayerList::begin()
{
    return layers.begin();
}

QList<RS_Layer*>::iterator RS_LayerList::end()
{
    return layers.end();
}

QList<RS_Layer*>::const_iterator RS_LayerList::begin()const
{
    return layers.begin();
}

QList<RS_Layer*>::const_iterator RS_LayerList::end()const
{
    return layers.end();
}

/**
 * Activates the given layer.
 * 
 * @param notify Notify listeners.
 */
void RS_LayerList::activate(const QString& name, bool notify) {
    RS_DEBUG->print("RS_LayerList::activate: %s, notify: %d begin",
                                    name.toLatin1().data(), notify);

    activate(find(name), notify);
    RS_DEBUG->print("RS_LayerList::activate: %s end", name.toLatin1().data());
}

/**
 * Activates the given layer.
 * 
 * @param notify Notify listeners.
 */
void RS_LayerList::activate(RS_Layer* layer, bool notify) {
    RS_DEBUG->print("RS_LayerList::activate notify: %d begin", notify);
    activeLayer = layer;
    RS_DEBUG->print("RS_LayerList::activate end");
}

/**
 * @brief sort by layer names
 */
void RS_LayerList::sort()
{
    std::stable_sort(layers.begin(), layers.end(), [](const RS_Layer* l0, const RS_Layer* l1 )->bool{
                         return l0->getName() < l1->getName();
                     });
}

/**
 * Adds a layer to the layer list.
 * If there is already a layer with the same name, no layer is 
 * added. In that case the layer passed to the method will be deleted!
 * If no layer was active so far, the new layer becomes the active one.
 *
 * Listeners are notified.
 */
void RS_LayerList::add(RS_Layer* layer) {
    RS_DEBUG->print("RS_LayerList::addLayer()");

    if (layer==nullptr) {
        return;
    }

    // check if layer already exists:
    RS_Layer* l = find(layer->getName());
    if (l==nullptr) {
        layers.append(layer);
        this->sort();

        // if there was no active layer so far, activate this one.
        if (activeLayer==nullptr) {
            activate(layer);
        }
    } else {
        // if there was no active layer so far, activate this one.
        if (activeLayer==nullptr) {
            activate(l);
        }

        l->freeze( layer->isFrozen());
        l->lock( layer->isLocked());
        l->setPrint( layer->isPrint());
        l->setConverted( layer->isConverted());
        l->setConstruction( layer->isConstruction());
        l->visibleInLayerList( layer->isVisibleInLayerList());
        l->setPen(layer->getPen());

        delete layer;
        layer = nullptr;
    }
}

/**
 * Removes a layer from the list.
 * Listeners are notified after the layer was removed from 
 * the list but before it gets deleted.
 */
void RS_LayerList::remove(RS_Layer* layer) {
    RS_DEBUG->print("RS_LayerList::removeLayer()");
    if (layer==nullptr) {
        return;
    }

    // here the layer is removed from the list but not deleted
    layers.removeOne(layer);

    // activate an other layer if necessary:
    if (activeLayer==layer) {
        activate(layers.first());
    }

    // now it's save to delete the layer
    delete layer;
}

/**
 * Changes a layer's attributes. The attributes of layer 'layer'
 * are copied from layer 'source'.
 * Listeners are notified.
 */
void RS_LayerList::edit(RS_Layer* layer, const RS_Layer& source) {
    if (layer==nullptr) {
        return;
    }

    *layer = source;
}

/**
 * @return Pointer to the layer with the given name or
 * \p nullptr if no such layer was found.
 */
RS_Layer* RS_LayerList::find(const QString& name) {
    //RS_DEBUG->print("RS_LayerList::find begin");

    RS_Layer* ret = nullptr;

    for (int i=0; i<layers.size(); ++i) {
        RS_Layer* l = layers.at(i);
        if (l->getName()==name) {
            ret = l;
            break;
        }
    }

    //RS_DEBUG->print("RS_LayerList::find end");

    return ret;
}

/**
 * @return Index of the given layer in the layer list or -1 if the layer
 * was not found.
 */
int RS_LayerList::getIndex(const QString& name) {
    //RS_DEBUG->print("RS_LayerList::find begin");

    int ret = -1;

    for (int i=0; i<layers.size(); i++) {
        RS_Layer* l = layers.at(i);

        if (l->getName()==name) {
            ret = i;
            break;
        }
    }

    //RS_DEBUG->print("RS_LayerList::find end");

    return ret;
}

/**
 * @return Index of the given layer in the layer list or -1 if the layer
 * was not found.
 */
int RS_LayerList::getIndex(RS_Layer* layer) {
    //RS_DEBUG->print("RS_LayerList::find begin");
    return layers.indexOf(layer);
}

/**
 * Switches on / off the given layer. 
 * Listeners are notified.
 */
void RS_LayerList::toggle(const QString& name) {
    toggle(find(name));
}

/**
 * Switches on / off the given layer. 
 * Listeners are notified.
 */
void RS_LayerList::toggle(RS_Layer* layer) {

    if (!layer) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_LayerList::toggle: nullptrptr layer");
        return;
    }

    // set flags
    layer->toggle();
}

/**
 * Locks or unlocks the given layer.
 * Listeners are notified.
 */
void RS_LayerList::toggleLock(RS_Layer* layer) {
    if (layer==nullptr) {
        return;
    }

    layer->toggleLock();
}

/**
 * Switch printing for the given layer on / off.
 * Listeners are notified.
 */
void RS_LayerList::togglePrint(RS_Layer* layer) {
    if (layer==nullptr) {
        return;
    }

    layer->togglePrint();
}

/**
 * Switch construction attribute for the given layer on / off.
 * Listeners are notified.
 */
void RS_LayerList::toggleConstruction(RS_Layer* layer) {
    if (layer==nullptr) {
        return;
    }

    layer->toggleConstruction();
}

/**
 * Locks or unlocks all layers.
 *
 * @param lock true: lock, false: unlock
 */
void RS_LayerList::lockAll(bool lock) {

    for (unsigned l=0; l<count(); l++) {
        if (at(l)->isVisibleInLayerList()) {
             at(l)->lock(lock);
         }
    }
}
