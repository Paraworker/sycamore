#ifndef SYCAMORE_SCENE_H
#define SYCAMORE_SCENE_H

#include "sycamore/defines.h"
#include "sycamore/utils/Point.h"
#include "sycamore/scene/SceneElement.h"
#include "sycamore/wlroots.h"

#include <memory>

NAMESPACE_SYCAMORE_BEGIN

class Scene {
public:
    using UPtr = std::unique_ptr<Scene>;

public:
    /**
     * @brief Create Scene
     * @return nullptr on failure
     */
    static Scene::UPtr create(wlr_output_layout* layout, wlr_presentation* presentation, wlr_linux_dmabuf_v1* dmabuf);

    ~Scene();

    auto getHandle() { return m_handle; }

    auto getLayout() { return m_sceneLayout; }

    wlr_scene_tree* getLayerTree(zwlr_layer_shell_v1_layer type) const;

    wlr_scene_node* nodeAt(const Point<double>& lCoords, Point<double>& sCoords) const {
        return wlr_scene_node_at(&shell.root->node, lCoords.x, lCoords.y, &sCoords.x, &sCoords.y);
    }

    static wlr_surface* surfaceFromNode(wlr_scene_node* node);

    static SceneElement* elementFromNode(wlr_scene_node* node);

    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

public:
    // tree structure:
    // - root
    //   - shell
    // 	   - background
    // 	   - bottom
    // 	   - view
    // 	   - top
    //     - overlay
    // 	 - dragIcons
    struct {
        wlr_scene_tree* root;
        wlr_scene_tree* background;
        wlr_scene_tree* bottom;
        wlr_scene_tree* view;
        wlr_scene_tree* top;
        wlr_scene_tree* overlay;
    } shell{};

    wlr_scene_tree* dragIcons{};

private:
    Scene(wlr_scene* handle, wlr_scene_output_layout* sceneLayout);

private:
    wlr_scene*               m_handle;
    wlr_scene_output_layout* m_sceneLayout;
};

NAMESPACE_SYCAMORE_END

#endif //SYCAMORE_SCENE_H