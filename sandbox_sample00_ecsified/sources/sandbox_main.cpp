/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/
#include <enduro2d/enduro2d.hpp>
#include <generated/sandbox_gen.h>
using namespace e2d;

namespace
{
    class game_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            E2D_UNUSED(owner);

            const keyboard& k = the<input>().keyboard();

            if ( k.is_key_just_released(keyboard_key::f12) ) {
                the<dbgui>().toggle_visible(!the<dbgui>().visible());
            }

            if ( k.is_key_just_released(keyboard_key::escape) ) {
                the<window>().set_should_close(true);
            }

            if ( k.is_key_pressed(keyboard_key::lalt) && k.is_key_just_released(keyboard_key::enter) ) {
                the<window>().toggle_fullscreen(!the<window>().fullscreen());
            }
        }
    };

    class camera_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            owner.for_joined_components<camera>(
            [](const ecs::const_entity&, camera& cam){
                if ( !cam.target() ) {
                    cam.viewport(
                        the<window>().real_size());
                    cam.projection(math::make_orthogonal_lh_matrix4(
                        the<window>().real_size().cast_to<f32>(), 0.f, 1000.f));
                }
            });
        }
    };

    struct ship_tag {};
    class sample_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            const auto time = the<engine>().time();
            owner.for_joined_components<ship_tag, renderer>(
                [&time](const ecs::const_entity&, const ship_tag& tag, renderer& r){
                    E2D_UNUSED(tag);
                    r.properties()
                        .property("u_time", time);
                });
        }
    };

    vector<u32> generate_quad_indices() noexcept {
        return {0, 1, 2, 2, 1, 3};
    }

    vector<v3f> generate_vertices() noexcept {
        const f32 hw = 33.0f;
        const f32 hh = 56.5f;
        return {
            {-hw,   hh, 0.0f},
            {-hw,  -hh, 0.0f},
            { hw,   hh, 0.0f},
            { hw,  -hh, 0.0f}
        };
    }

    vector<v2f> generate_uvs() noexcept {
        return {
            {0, 1},
            {0, 0},

            {1, 1},
            {1, 0}
        };
    }

    vector<color32> generate_colors() noexcept {
        return {
            color32::red(),
            color32::green(),
            color32::blue(),
            color32::yellow()
        };
    }

    struct game final : public high_application {
        bool initialize() final {
            return create_scene()
                && create_camera()
                && create_systems();
        }
    private:

        bool create_scene() {
            auto ship_mat = the<library>().load_asset<material_asset>("material.json");
            if(!ship_mat)
                return false;

            mesh ship_mesh;
            ship_mesh.set_vertices(generate_vertices());
            ship_mesh.set_indices(0, generate_quad_indices());
            ship_mesh.set_colors(0, generate_colors());
            ship_mesh.set_uvs(0, generate_uvs());
            auto ship_mesh_asset = mesh_asset::create(ship_mesh);
            if(!ship_mesh_asset)
                return false;

            model ship_model;
            ship_model.set_mesh(ship_mesh_asset);
            ship_model.regenerate_geometry(the<render>());
            auto ship_model_asset = model_asset::create(ship_model);
            if(!ship_model_asset)
                return false;

            ecs::entity scene_e = the<world>().registry().create_entity();
            scene_e.assign_component<scene>(node::create(the<world>()));
            node_iptr scene_r = scene_e.get_component<scene>().root();

            {
                ecs::entity model_e = the<world>().registry().create_entity();
                ecs::entity_filler(model_e)
                    .component<actor>(node::create(model_e, scene_r))
                    .component<renderer>(renderer()
                        .materials({ship_mat}))
                    .component<model_renderer>(ship_model_asset)
                    .component<ship_tag>();
            }

            return true;
        }

        bool create_camera() {
            ecs::entity camera_e = the<world>().registry().create_entity();
            ecs::entity_filler(camera_e)
                .component<camera>(camera()
                    .background({1.f, 0.4f, 0.f, 1.f}))
                .component<actor>(node::create(camera_e));
            return true;
        }

        bool create_systems() {
            ecs::registry_filler(the<world>().registry())
                .system<game_system>(world::priority_update)
                .system<camera_system>(world::priority_pre_render)
                .system<sample_system>(world::priority_render-1);
            return true;
        }
    };
}

int e2d_main(int argc, char *argv[]) {
    const auto starter_params = starter::parameters(
        engine::parameters(SANDBOX_NAME, "enduro2d")
            .timer_params(engine::timer_parameters()
                .maximal_framerate(100)))
            .library_root(url{"resources://" SANDBOX_DATA_DIR});

    modules::initialize<starter>(argc, argv, starter_params).start<game>();
    modules::shutdown<starter>();
    return 0;
}
