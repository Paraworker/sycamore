#ifndef SYCAMORE_SCENE_H
#define SYCAMORE_SCENE_H

#include "sycamore/utils/Point.h"
#include "sycamore/scene/Element.h"
#include "sycamore/wlroots.h"

namespace sycamore::scene
{

struct Scene
{
    // shell tree structure
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
    Scene();

    /**
     * @brief Destructor
     */
    ~Scene();

    /**
     * @brief Init Scene
     */
    void init(wlr_output_layout* outputLayout, wlr_linux_dmabuf_v1* dmabuf);

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

    /**
     * @brief  Add an output to scene layout
     * @return scene output
     */
    wlr_scene_output* addOutput(wlr_output* outputHandle, wlr_output_layout_output* layoutOutput);

    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

private:
    wlr_scene_output_layout* layout;
};

wlr_surface* surfaceFromNode(wlr_scene_node* node);

Element* elementFromNode(wlr_scene_node* node);

}

#endif //SYCAMORE_SCENE_H