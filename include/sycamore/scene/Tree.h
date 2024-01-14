#ifndef SYCAMORE_SCENE_TREE_H
#define SYCAMORE_SCENE_TREE_H

#include "sycamore/utils/Point.h"
#include "sycamore/scene/Element.h"
#include "sycamore/wlroots.h"

namespace sycamore::scene
{

// scene tree
struct Tree
{
    // shell tree
    struct Shell
    {
        wlr_scene_tree* root;
        wlr_scene_tree* background;
        wlr_scene_tree* bottom;
        wlr_scene_tree* toplevel;
        wlr_scene_tree* top;
        wlr_scene_tree* overlay;
    };

    wlr_scene*      root;
    Shell           shell;
    wlr_scene_tree* dragIcons;

    /**
     * @brief Constructor
     */
    Tree();

    /**
     * @brief Destructor
     */
    ~Tree();

    /**
     * @brief Get scene tree for layer
     */
    wlr_scene_tree* treeForLayer(zwlr_layer_shell_v1_layer type) const;

    /**
     * @brief Find the topmost node in shell tree
     */
    wlr_scene_node* shellAt(const Point<double>& lCoords, Point<double>& sCoords) const
    {
        return wlr_scene_node_at(&shell.root->node, lCoords.x, lCoords.y, &sCoords.x, &sCoords.y);
    }

    Tree(const Tree&) = delete;
    Tree(Tree&&) = delete;
    Tree& operator=(const Tree&) = delete;
    Tree& operator=(Tree&&) = delete;
};

wlr_surface* surfaceFromNode(wlr_scene_node* node);

Element* elementFromNode(wlr_scene_node* node);

}

#endif //SYCAMORE_SCENE_TREE_H