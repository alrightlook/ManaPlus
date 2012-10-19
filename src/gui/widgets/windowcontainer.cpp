/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/widgets/windowcontainer.h"

#include "gui/widgets/window.h"

#include "utils/dtor.h"

#include "debug.h"

WindowContainer *windowContainer = nullptr;

WindowContainer::WindowContainer(const Widget2 *const widget) :
    Container(widget)
{
}

void WindowContainer::logic()
{
    delete_all(mDeathList);
    mDeathList.clear();

    gcn::Container::logic();
}

void WindowContainer::scheduleDelete(gcn::Widget *const widget)
{
    if (widget)
        mDeathList.push_back(widget);
}

void WindowContainer::adjustAfterResize(const int oldScreenWidth,
                                        const int oldScreenHeight)
{
    for (WidgetListIterator i = mWidgets.begin(), i_end = mWidgets.end();
         i != i_end; ++i)
    {
        if (Window *const window = dynamic_cast<Window*>(*i))
            window->adjustPositionAfterResize(oldScreenWidth, oldScreenHeight);
    }
}
