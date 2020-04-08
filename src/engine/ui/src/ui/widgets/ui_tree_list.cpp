#include <utility>


#include "widgets/ui_tree_list.h"

#include "widgets/ui_image.h"
#include "widgets/ui_label.h"
using namespace Halley;


UITreeList::UITreeList(String id, UIStyle style)
	: UIList(std::move(id), std::move(style))
{
	setupEvents();
}

void UITreeList::addTreeItem(const String& id, const String& parentId, const LocalisedString& label)
{
	auto listItem = std::make_shared<UIListItem>(id, *this, style.getSubStyle("item"), int(getNumberOfItems()), style.getBorder("extraMouseBorder"));

	// Controls
	const auto treeControls = std::make_shared<UITreeListControls>(id, style.getSubStyle("controls"));
	listItem->add(treeControls, 0, {}, UISizerFillFlags::Fill);

	// Icon
	//auto icon = std::make_shared<UIImage>(style.getSubStyle("controls").getSprite("element"));
	//listItem->add(icon, 0, {}, UISizerAlignFlags::Centre);

	// Label
	auto labelWidget = std::make_shared<UILabel>(id + "_label", style.getTextRenderer("label"), label);
	if (style.hasTextRenderer("selectedLabel")) {
		labelWidget->setSelectable(style.getTextRenderer("label"), style.getTextRenderer("selectedLabel"));
	}
	if (style.hasTextRenderer("disabledStyle")) {
		labelWidget->setDisablable(style.getTextRenderer("label"), style.getTextRenderer("disabledStyle"));
	}
	listItem->add(labelWidget, 0, style.getBorder("labelBorder"), UISizerFillFlags::Fill);
	listItem->setDraggableSubWidget(labelWidget.get());

	// Logical item
	auto treeItem = UITreeListItem(id, listItem, treeControls);
	auto& parentItem = getItemOrRoot(parentId);
	parentItem.addChild(std::move(treeItem));

	addItem(listItem, Vector4f(), UISizerAlignFlags::Left | UISizerFillFlags::FillVertical);
}

void UITreeList::clear()
{
	UIList::clear();
	root = UITreeListItem();
}

void UITreeList::update(Time t, bool moved)
{
	UIList::update(t, moved);
	root.updateTree(*this);
}

void UITreeList::draw(UIPainter& painter) const
{
	UIList::draw(painter);
	if (insertCursor.hasMaterial()) {
		painter.draw(insertCursor);
	}
}

void UITreeList::onItemDragging(UIListItem& item, int index, Vector2f pos)
{
	auto elem = root.tryFindId(item.getId());
	if (elem) {
		elem->setExpanded(false);
	}

	const auto res = root.findPosition(pos + item.getRect().getSize() / 2);
	if (res) {
		const auto& resData = res.value();
		auto rect = resData.rect;
		if (rect.getHeight() < 1) {
			rect = Rect4f(rect.getTopLeft() - Vector2f(0, 1), rect.getTopRight() + Vector2f(0, 1));
		}
		
		insertCursor = style.getSubStyle("cursor").getSprite(resData.type == UITreeListItem::PositionType::OnTop ? "over" : "beforeAfter");
		insertCursor.setPos(rect.getTopLeft()).scaleTo(rect.getSize());
	}	
}

void UITreeList::onItemDoneDragging(UIListItem& item, int index, Vector2f pos)
{
	auto res = root.findPosition(pos + item.getRect().getSize() / 2);
	if (res) {
		const auto& resData = res.value();

		const String& itemId = item.getId();
		String newParentId;
		size_t newChildIndex;
		
		if (resData.type == UITreeListItem::PositionType::OnTop) {
			newParentId = resData.item->getId();
			newChildIndex = resData.item->getNumberOfChildren();
		} else {
			newParentId = resData.item->getParentId();
			const auto parent = root.tryFindId(newParentId);
			const auto siblingIndex = parent->getChildIndex(resData.item->getId());
			newChildIndex = siblingIndex + (resData.type == UITreeListItem::PositionType::Before ? 0 : 1);
		}

		const auto& curItem = *root.tryFindId(itemId);
		const String& oldParentId = curItem.getParentId();
		auto& oldParent = *root.tryFindId(oldParentId);
		const size_t oldChildIndex = oldParent.getChildIndex(itemId);

		if (oldParentId != newParentId || oldChildIndex != newChildIndex) {
			// TODO: swap this without rebuilding whole thing?
			/*
			if (oldParentId == newParentId) {
				oldParent.moveChild(oldChildIndex, newChildIndex);
			} else {
				auto& newParent = *root.tryFindId(newParentId);
				newParent.addChild(oldParent.removeChild(itemId), newChildIndex);
			}
			reassignIds();
			*/			

			sendEvent(UIEvent(UIEventType::TreeItemReparented, getId(), itemId, newParentId, int(newChildIndex)));
		}
	}
	insertCursor = Sprite();
}

UITreeListItem& UITreeList::getItemOrRoot(const String& id)
{
	const auto res = root.tryFindId(id);
	if (res) {
		return *res;
	}
	return root;
}

void UITreeList::setupEvents()
{
	setHandle(UIEventType::TreeCollapse, [=] (const UIEvent& event)
	{
		auto elem = root.tryFindId(event.getStringData());
		if (elem) {
			elem->setExpanded(false);
		}
	});

	setHandle(UIEventType::TreeExpand, [=](const UIEvent& event)
	{
		auto elem = root.tryFindId(event.getStringData());
		if (elem) {
			elem->setExpanded(true);
		}
	});
}

UITreeListControls::UITreeListControls(String id, UIStyle style)
	: UIWidget(std::move(id), Vector2f(), UISizer(UISizerType::Horizontal, 0))
	, style(std::move(style))
{
	setupUI();
}

float UITreeListControls::updateGuides(const std::vector<int>& itemsLeftPerDepth, bool hasChildren)
{
	if (waitingConstruction || itemsLeftPerDepth.size() != lastDepth) {
		clear();
		guides.clear();
		lastDepth = itemsLeftPerDepth.size();

		auto getSprite = [&](size_t depth) -> Sprite
		{
			const bool leaf = depth == itemsLeftPerDepth.size();
			if (leaf) {
				return style.getSprite("leaf");
			} else {
				const bool deepest = depth == itemsLeftPerDepth.size() - 1;
				const auto left = itemsLeftPerDepth[depth];
				if (deepest) {
					if (left == 1) {
						return style.getSprite("guide_l");
					} else {
						return style.getSprite("guide_t");
					}
				} else {
					if (left == 1) {
						return Sprite().setSize(Vector2f(22, 22));
					} else {
						return style.getSprite("guide_i");
					}
				}
			}
		};
		
		for (size_t i = 1; i < itemsLeftPerDepth.size() + (hasChildren ? 0 : 1); ++i) {
			guides.push_back(std::make_shared<UIImage>(getSprite(i)));
			add(guides.back(), 0, Vector4f(0, -1, 0, 0));
		}

		if (hasChildren) {
			expandButton = std::make_shared<UIButton>("expand", style.getSubStyle("expandButton"));
			collapseButton = std::make_shared<UIButton>("collapse", style.getSubStyle("collapseButton"));
			expandButton->setActive(false);
			add(expandButton, 0, Vector4f(), UISizerAlignFlags::Centre);
			add(collapseButton, 0, Vector4f(), UISizerAlignFlags::Centre);
		}

		totalIndent = getLayoutMinimumSize(false).x;
		waitingConstruction = false;
	}

	return totalIndent;
}

void UITreeListControls::setExpanded(bool expanded)
{
	if (expandButton) {
		expandButton->setActive(!expanded);
	}
	if (collapseButton) {
		collapseButton->setActive(expanded);
	}
}

void UITreeListControls::setupUI()
{
	setHandle(UIEventType::ButtonClicked, "expand", [=] (const UIEvent& event)
	{
		sendEvent(UIEvent(UIEventType::TreeExpand, getId(), getId()));
	});
	setHandle(UIEventType::ButtonClicked, "collapse", [=](const UIEvent& event)
	{
		sendEvent(UIEvent(UIEventType::TreeCollapse, getId(), getId()));
	});
}

UITreeListItem::UITreeListItem() = default;

UITreeListItem::UITreeListItem(String id, std::shared_ptr<UIListItem> listItem, std::shared_ptr<UITreeListControls> treeControls)
	: id(std::move(id))
	, listItem(std::move(listItem))
	, treeControls(std::move(treeControls))
{}

UITreeListItem* UITreeListItem::tryFindId(const String& id)
{
	if (id == this->id) {
		return this;
	}

	for (auto& c: children) {
		const auto res = c.tryFindId(id);
		if (res) {
			return res;
		}
	}

	return nullptr;
}

void UITreeListItem::addChild(UITreeListItem item)
{
	if (children.empty()) {
		expanded = true;
	}
	item.parentId = id;
	children.emplace_back(std::move(item));
}

void UITreeListItem::addChild(UITreeListItem item, size_t pos)
{
	if (children.empty()) {
		expanded = true;
	}
	item.parentId = id;
	children.insert(children.begin() + pos, std::move(item));
}

UITreeListItem UITreeListItem::removeChild(const String& id)
{
	const size_t n = children.size();
	for (size_t i = 0; i < n; ++i) {
		if (children[i].id == id) {
			UITreeListItem item = std::move(children[i]);
			children.erase(children.begin() + i);
			item.parentId = "";
			return item;
		}
	}
	throw Exception("No child with id \"" + id + "\"", HalleyExceptions::UI);
}

void UITreeListItem::moveChild(size_t oldChildIndex, size_t newChildIndex)
{
	if (newChildIndex < oldChildIndex) {
		std::rotate(children.begin() + newChildIndex, children.begin() + oldChildIndex, children.begin() + oldChildIndex);
	} else {
		std::rotate(children.begin() + oldChildIndex, children.begin() + newChildIndex, children.begin() + newChildIndex);
	}
}

void UITreeListItem::setExpanded(bool e)
{
	if (!children.empty()) {
		expanded = e;
		treeControls->setExpanded(e);
	}
}

std::optional<UITreeListItem::FindPositionResult> UITreeListItem::findPosition(Vector2f pos) const
{
	if (listItem) {
		const auto r = listItem->getRect();
		const auto b = listItem->getClickableInnerBorder();
		const float x0 = r.getLeft() + b.x;
		const float x1 = r.getRight() - b.z;
		const float y0 = r.getTop() + b.y;
		const float y1 = r.getBottom() - b.w + 1;
		const float h = y1 - y0;
		const float y = pos.y;
		
		if (y >= y0 && y < y1) {
			if (y < y0 + h / 4) {
				return FindPositionResult(PositionType::Before, this, Rect4f(x0, y0, x1 - x0, 0));
			} else if (y > y0 + 3 * h / 4) {
				return FindPositionResult(PositionType::After, this, Rect4f(x0, y1, x1 - x0, 0));
			} else {
				return FindPositionResult(PositionType::OnTop, this, Rect4f(x0, y0, x1 - x0, y1 - y0));
			}
		}
	}

	for (auto& c: children) {
		auto res = c.findPosition(pos);
		if (res) {
			return *res;
		}
	}
	
	return {};
}

const String& UITreeListItem::getId() const
{
	return id;
}

const String& UITreeListItem::getParentId() const
{
	return parentId;
}

size_t UITreeListItem::getNumberOfChildren() const
{
	return children.size();
}

size_t UITreeListItem::getChildIndex(const String& id) const
{
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i].id == id) {
			return i;
		}
	}
	return 0;
}

void UITreeListItem::updateTree(UITreeList& treeList)
{
	std::vector<int> itemsLeftPerDepth;
	doUpdateTree(treeList, itemsLeftPerDepth, expanded);
}

void UITreeListItem::doUpdateTree(UITreeList& treeList, std::vector<int>& itemsLeftPerDepth, bool treeExpanded)
{
	treeList.setItemActive(id, treeExpanded);

	if (listItem && treeControls && treeExpanded) {
		const float totalIndent = treeControls->updateGuides(itemsLeftPerDepth, !children.empty());
		listItem->setClickableInnerBorder(Vector4f(totalIndent, 0, 0, 0));
	}
	
	itemsLeftPerDepth.push_back(int(children.size()));
	for (auto& c: children) {
		c.doUpdateTree(treeList, itemsLeftPerDepth, expanded && treeExpanded);
		itemsLeftPerDepth.back()--;
	}
	itemsLeftPerDepth.pop_back();
}
