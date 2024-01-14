#include "sycamore/scene/Tree.h"

#include <stdexcept>

namespace sycamore::scene
{

struct RootElement final : Element
{
    explicit RootElement(wlr_scene_node* node)
        : Element(ROOT, node) {}

    ~RootElement() override = default;
};

Tree::Tree()
    : root{nullptr}
    , shell{nullptr}
    , dragIcons{nullptr}
{
    if (root = wlr_scene_create(); !root)
    {
        throw std::runtime_error("Create wlr_scene failed!");
    }

    shell.root = wlr_scene_tree_create(&root->tree);

    shell.background = wlr_scene_tree_create(shell.root);
    shell.bottom     = wlr_scene_tree_create(shell.root);
    shell.toplevel   = wlr_scene_tree_create(shell.root);
    shell.top        = wlr_scene_tree_create(shell.root);
    shell.overlay    = wlr_scene_tree_create(shell.root);

    dragIcons = wlr_scene_tree_create(&root->tree);

    // Create RootElement
    new RootElement{&root->tree.node};
}

Tree::~Tree()
{
    wlr_scene_node_destroy(&root->tree.node);
}

wlr_scene_tree* Tree::treeForLayer(zwlr_layer_shell_v1_layer type) const
{
    switch (type)
    {
        case ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND:
            return shell.background;
        case ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM:
            return shell.bottom;
        case ZWLR_LAYER_SHELL_V1_LAYER_TOP:
            return shell.top;
        case ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY:
            return shell.overlay;
        default:
            throw std::logic_error("unreachable!");
    }
}

wlr_surface* surfaceFromNode(wlr_scene_node* node)
{
    if (!node || node->type != WLR_SCENE_NODE_BUFFER)
    {
        return {};
    }

    auto sceneSurface = wlr_scene_surface_try_from_buffer(wlr_scene_buffer_from_node(node));
    if (!sceneSurface)
    {
        return {};
    }

    return sceneSurface->surface;
}

Element* elementFromNode(wlr_scene_node* node)
{
    if (!node)
    {
        return {};
    }

    wlr_scene_tree* tree;

    switch (node->type)
    {
        case WLR_SCENE_NODE_TREE:
            tree = wlr_scene_tree_from_node(node);
            break;
        case WLR_SCENE_NODE_RECT:
        case WLR_SCENE_NODE_BUFFER:
            tree = node->parent;
            break;
    }

    while (!tree->node.data)
    {
        tree = tree->node.parent;
    }

    return static_cast<Element*>(tree->node.data);
}

}