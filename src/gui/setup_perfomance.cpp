/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  Andrei Karas
 *  Copyright (C) 2011  The ManaPlus Developers
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/setup_perfomance.h"

#include "gui/chatwindow.h"
#include "gui/editdialog.h"
#include "gui/setupitem.h"

#include "gui/widgets/button.h"
#include "gui/widgets/chattab.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/scrollarea.h"

#include "configuration.h"
#include "localplayer.h"
#include "log.h"

#include "utils/gettext.h"

#include "debug.h"

Setup_Perfomance::Setup_Perfomance()
{
    setName(_("Perfomance"));

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);
    place(0, 0, mScroll, 10, 10);

    new SetupItemLabel(_("Better perfomance (enable for better perfomance)"),
        "", this);

    new SetupItemCheckBox(_("Auto adjust perfomance"), "",
        "adjustPerfomance", this, "adjustPerfomanceEvent");

    new SetupItemCheckBox(_("Show beings transparency"), "",
        "beingopacity", this, "beingopacityEvent");

    new SetupItemCheckBox(_("Hw acceleration"), "",
        "hwaccel", this, "hwaccelEvent");

    new SetupItemCheckBox(_("Enable opacity cache (Software, can "
        "use many memory)"), "", "alphaCache", this, "alphaCacheEvent");

    new SetupItemCheckBox(_("Enable map reduce (Software)"), "",
        "enableMapReduce", this, "enableMapReduceEvent");

    new SetupItemLabel(_("Better quality (disable for better perfomance)"),
        "", this);

    new SetupItemCheckBox(_("Enable alpha channel fix (Software, can "
        "be very slow)"), "Can slow down drawing", "enableAlphaFix",
        this, "enableAlphaFixEvent");

    new SetupItemCheckBox(_("Enable reorder sprites."), "",
        "enableReorderSprites", this, "enableReorderSpritesEvent");

    new SetupItemLabel(_("Small memory (enable for lower memory usage)"),
         "", this);

    new SetupItemCheckBox(_("Disable advanced beings caching (Software)"), "",
        "disableAdvBeingCaching", this, "disableAdvBeingCachingEvent");

    new SetupItemCheckBox(_("Disable beings caching (Software)"), "",
        "disableBeingCaching", this, "disableBeingCachingEvent");

    setDimension(gcn::Rectangle(0, 0, 550, 350));
}

void Setup_Perfomance::apply()
{
    SetupTabScroll::apply();
}
