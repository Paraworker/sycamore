#include "sycamore/scene/Scene.h"
#include <spdlog/spdlog.h>

namespace sycamore
{

class RootElement final : public SceneElement
{
public:
    explicit RootElement(wlr_scene_node* node)
        : SceneElement(SceneElement::ROOT, node) {}

    ~RootElement() override = default;
};

Scene::UPtr Scene::create(wlr_output_layout* layout, wlr_presentation* presentation, wlr_linux_dmabuf_v1* dmabuf)
{
    auto handle = wlr_scene_create();
    if (!handle)
    {
        spdlog::error("Create wlr_scene failed");
        return {};
    }

    auto sceneLayout = wlr_scene_attach_output_layout(handle, layout);
    if (!sceneLayout)
    {
        spdlog::error("wlr_scene attach wlr_output_layout failed");
        return {};
    }

    wlr_scene_set_presentation(handle, presentation);
    wlr_scene_set_linux_dmabuf_v1(handle, dmabuf);

    return UPtr{new Scene{handle, sceneLayout}};
}

Scene::Scene(wlr_scene* handle, wlr_scene_output_layout* sceneLayout)
    : m_handle{handle}, m_sceneLayout{sceneLayout}
{
    // Create trees
    shell.root = wlr_scene_tree_create(&m_handle->tree);

    shell.background = wlr_scene_tree_create(shell.root);
    shell.bottom     = wlr_scene_tree_create(shell.root);
    shell.toplevel   = wlr_scene_tree_create(shell.root);
    shell.top        = wlr_scene_tree_create(shell.root);
    shell.overlay    = wlr_scene_tree_create(shell.root);

    dragIcons = wlr_scene_tree_create(&m_handle->tree);

    // Create RootElement
    new RootElement{&handle->tree.node};
}

Scene::~Scene()
{
    wlr_scene_node_destroy(&m_handle->tree.node);
}

wlr_scene_tree* Scene::getLayerTree(zwlr_layer_shell_v1_layer type) const
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
            return {};
    }
}

wlr_surface* Scene::surfaceFromNode(wlr_scene_node* node)
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

SceneElement* Scene::elementFromNode(wlr_scene_node* node)
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

    return static_cast<SceneElement*>(tree->node.data);
}

}