#include "sycamore/scene/Scene.h"

#include <stdexcept>

namespace sycamore::scene
{

struct RootElement final : Element
{
    Listener destroy;

    explicit RootElement(wlr_scene_node& node)
        : Element{ROOT}
    {
        // attach node
        node.data = this;

        destroy = [this](auto)
        {
            delete this;
        };
        destroy.connect(node.events.destroy);
    }

    ~RootElement() = default;
};

Scene::Scene() : root{wlr_scene_create()}, shell{}, dragIcons{}, layout{}
{
    new RootElement{root->tree.node};

    shell.root = wlr_scene_tree_create(&root->tree);

    shell.background = wlr_scene_tree_create(shell.root);
    shell.bottom     = wlr_scene_tree_create(shell.root);
    shell.toplevel   = wlr_scene_tree_create(shell.root);
    shell.top        = wlr_scene_tree_create(shell.root);
    shell.overlay    = wlr_scene_tree_create(shell.root);

    dragIcons = wlr_scene_tree_create(&root->tree);
}

Scene::~Scene()
{
    wlr_scene_node_destroy(&root->tree.node);
}

void Scene::init(wlr_output_layout* outputLayout, wlr_linux_dmabuf_v1* dmabuf)
{
    layout = wlr_scene_attach_output_layout(root, outputLayout);
    wlr_scene_set_linux_dmabuf_v1(root, dmabuf);
}

wlr_scene_tree* Scene::treeForLayer(zwlr_layer_shell_v1_layer type) const
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
            throw std::logic_error{"unreachable!"};
    }
}

wlr_scene_output* Scene::addOutput(wlr_output* outputHandle, wlr_output_layout_output* layoutOutput)
{
    const auto sceneOutput = wlr_scene_output_create(root, outputHandle);
    wlr_scene_output_layout_add_output(layout, layoutOutput, sceneOutput);
    return sceneOutput;
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