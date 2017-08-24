/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2017  The ManaPlus Developers
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

#include "net/eathena/mail2recv.h"

#include "itemcolormanager.h"
#include "logger.h"
#include "notifymanager.h"

#include "const/net/inventory.h"

#include "const/resources/item/cards.h"

#include "being/playerinfo.h"

#include "enums/resources/notifytypes.h"

#include "gui/windows/maileditwindow.h"
#include "gui/windows/mailwindow.h"

#include "net/messagein.h"

#include "net/eathena/mail2handler.h"

#include "resources/iteminfo.h"

#include "resources/db/itemdb.h"

#include "resources/inventory/inventory.h"

#include "resources/item/item.h"
#include "resources/item/itemoptionslist.h"

#include "utils/checkutils.h"
#include "utils/gettext.h"

#include "debug.h"

namespace EAthena
{

namespace Mail2Recv
{
    std::queue<MailQueue*> mMailQueue;
}  // namespace Mail2Recv

void Mail2Recv::processMailIcon(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readUInt8("show icon");
}

void Mail2Recv::processOpenNewMailWindow(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readString(24, "receiver");
    msg.readUInt8("result");
}

void Mail2Recv::processAddItemResult(Net::MessageIn &msg)
{
    const int res = msg.readUInt8("result");
    const int index = msg.readInt16("index") - INVENTORY_OFFSET;
    const int amount = msg.readInt16("amount");
    const int itemId = msg.readInt16("item id");
    const ItemTypeT itemType = static_cast<ItemTypeT>(
        msg.readUInt8("item type"));
    const uint8_t identify = msg.readUInt8("identify");
    const Damaged damaged = fromBool(msg.readUInt8("attribute"), Damaged);
    const uint8_t refine = msg.readUInt8("refine");
    int cards[maxCards];
    for (int f = 0; f < maxCards; f++)
        cards[f] = msg.readUInt16("card");
    ItemOptionsList *options = new ItemOptionsList(5);
    for (int f = 0; f < 5; f ++)
    {
        const uint16_t idx = msg.readInt16("option index");
        const uint16_t val = msg.readInt16("option value");
        msg.readUInt8("option param");
        options->add(idx, val);
    }
    msg.readInt16("weight");
    msg.readUInt8("unknown 1");
    msg.readUInt8("unknown 2");
    msg.readUInt8("unknown 3");
    msg.readUInt8("unknown 4");
    msg.readUInt8("unknown 5");

    if (mailEditWindow == nullptr)
    {
        reportAlways("Mail edit window not created");
        delete options;
        return;
    }
    Inventory *const inventory = mailEditWindow->getInventory();
    if (inventory == nullptr)
    {
        reportAlways("Mail edit window inventory not exists");
        delete options;
        return;
    }

    if (res != 0)
    {
        switch (res)
        {
            case 1:
                NotifyManager::notify(
                    NotifyTypes::MAIL_ATTACH_ITEM_WEIGHT_ERROR);
                break;
            case 2:
                NotifyManager::notify(
                    NotifyTypes::MAIL_ATTACH_ITEM_FATAL_ERROR);
                break;
            case 3:
                NotifyManager::notify(
                    NotifyTypes::MAIL_ATTACH_ITEM_NO_SPACE);
                break;
            case 4:
                NotifyManager::notify(
                    NotifyTypes::MAIL_ATTACH_ITEM_NOT_TRADEABLE);
                break;
            default:
                NotifyManager::notify(
                    NotifyTypes::MAIL_ATTACH_ITEM_UNKNOWN_ERROR);
                UNIMPLEMENTEDPACKETFIELD(res);
                break;
        }
        delete options;
        return;
    }
    const int slot = inventory->addItem(itemId,
        itemType,
        amount,
        refine,
        ItemColorManager::getColorFromCards(&cards[0]),
        fromBool(identify, Identified),
        damaged,
        Favorite_false,
        Equipm_false,
        Equipped_false);
    if (slot == -1)
    {
        delete options;
        return;
    }
    inventory->setCards(slot, cards, 4);
    inventory->setOptions(slot, options);
    inventory->setTag(slot, index);
    delete options;
}

void Mail2Recv::processRemoveItemResult(Net::MessageIn &msg)
{
    const int result = msg.readUInt8("result");
    const int index = msg.readInt16("index") - INVENTORY_OFFSET;
    const int amount = msg.readInt16("count");
    msg.readInt16("weight");

    if (result == 0)
    {
        Inventory *const inv = PlayerInfo::getInventory();
        if (inv == nullptr)
        {
            reportAlways("Player inventory not exists");
            return;
        }
        std::string itemName;
        const Item *const item = inv->getItem(index);
        if (item != nullptr)
        {
            itemName = item->getName();
        }
        else
        {
            // TRANSLATORS: unknown item name
            itemName = _("Unknown item");
        }

        NotifyManager::notify(
            NotifyTypes::MAIL_REMOVE_ITEM_ERROR,
            itemName);
        return;
    }
    if (mailEditWindow == nullptr)
    {
        reportAlways("Mail edit window not created");
        return;
    }
    Inventory *const inventory = mailEditWindow->getInventory();
    if (inventory == nullptr)
    {
        reportAlways("Mail edit window inventory not exists");
        return;
    }
    const int index2 = inventory->findIndexByTag(index);
    if (index2 == -1)
    {
        reportAlways("Item not exists in mail edit window.");
        return;
    }
    Item *const item = inventory->getItem(index2);
    if (item == nullptr)
    {
        reportAlways("Item not exists.");
        return;
    }

    item->increaseQuantity(-amount);
}

void Mail2Recv::processCheckNameResult(Net::MessageIn &msg)
{
    const int charId = msg.readInt32("char id");
    msg.readInt16("class");
    msg.readInt16("level");
    if (msg.getVersion() >= 20160316)
        msg.readString(24, "name");
    // +++ in future if name received, need use it in map
    if (mMailQueue.empty())
    {
        reportAlways("Mail2Recv::processCheckNameResult no names in queue."
            "Char id: %d", charId);
        return;
    }
    MailQueue *const mail = mMailQueue.front();
    mMailQueue.pop();
    if (charId == 0)
    {
        NotifyManager::notify(NotifyTypes::MAIL_NAME_VALIDATION_ERROR,
            mail->to);
        delete mail;
        return;
    }
    switch (mail->type)
    {
        case MailQueueType::SendMail:
            mail2Handler->sendMail(mail->to,
                mail->title,
                mail->body,
                mail->money);
            break;
        case MailQueueType::EditMail:
            mailWindow->createMail(mail->to);
            break;
        case MailQueueType::Unknown:
        case MailQueueType::ValidateTO:
        default:
            reportAlways("Not implemented yet.");
            break;
    }
    delete mail;
}

void Mail2Recv::processSendResult(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readUInt8("result");
}

void Mail2Recv::processMailListPage(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readInt16("len");
    msg.readUInt8("open type");
    const int cnt = msg.readUInt8("cnt");
    msg.readUInt8("isEnd");
    for (int f = 0; f < cnt; f ++)
    {
        msg.readInt64("mail id");
        msg.readUInt8("is read");
        msg.readUInt8("type");
        msg.readString(24, "sender name");
        msg.readInt32("reg time");
        msg.readInt32("expire time");
        msg.readString(-1, "title");
    }
}

void Mail2Recv::processReadMail(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readInt16("len");
    msg.readUInt8("open type");
    msg.readInt64("mail id");
    const int textLen = msg.readInt16("text len");
    msg.readInt64("money");
    const int itemsCount = msg.readUInt8("item count");
    msg.readString(textLen, "text message");
    for (int f = 0; f < itemsCount; f ++)
    {
        msg.readInt16("amount");
        msg.readInt16("item id");
        msg.readUInt8("identify");
        msg.readUInt8("damaged");
        msg.readUInt8("refine");
        for (int d = 0; d < maxCards; d ++)
            msg.readUInt16("card");
        msg.readInt32("unknown");
        msg.readUInt8("type");
        msg.readInt32("unknown");
        for (int d = 0; d < 5; d ++)
        {
            msg.readInt16("option index");
            msg.readInt16("option value");
            msg.readUInt8("option param");
        }
    }
}

void Mail2Recv::processMailDelete(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readUInt8("open type");
    msg.readInt64("mail id");
}

void Mail2Recv::processRequestMoney(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readInt64("mail id");
    msg.readUInt8("open type");
    msg.readUInt8("result");
}

void Mail2Recv::processRequestItems(Net::MessageIn &msg)
{
    UNIMPLEMENTEDPACKET;
    msg.readInt64("mail id");
    msg.readUInt8("open type");
    msg.readUInt8("result");
}

}  // namespace EAthena