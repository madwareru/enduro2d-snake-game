#include <enduro2d/enduro2d.hpp>
#include <generated/sandbox_gen.h>
#include <shared.hpp>
#include "life_map.hpp"
#include <cstdlib>
#include <ctime>
using namespace e2d;

namespace
{
    struct life_data {
        static const u8 width = map_definition::map_width;
        static const u8 height = map_definition::map_height;
        ecs::entity_id foreground_e_id;
        float last_generation_time;
        bool is_dirty;
        std::array<u8, map_definition::map_width*map_definition::map_height> background_layer;
        std::array<bool, map_definition::map_width*map_definition::map_height> living_layer;
        std::array<bool, map_definition::map_width*map_definition::map_height> living_buffer;
    };

    template<typename Ft>
    void generate_tilemap_indices_internal(const life_data& ld,
                                           vector<u32>& dest,
                                           Ft&& pred) {
        static const u32 pattern[6] = {0, 1, 2, 2, 1, 3};
        u32 quad_off = 0;
        for(size_t j = 0; j < ld.height; ++j){
            for(size_t i = 0; i < ld.width; ++i) {
                if(!pred(ld, i, j)) {
                    continue;
                }
                for(size_t k = 0; k < 6; ++k) {
                    dest.push_back(quad_off + pattern[k]);
                }
                quad_off += 4;
            }
        }
    }

    vector<u32> generate_foreground_indices(const life_data& ld) noexcept {
        vector<u32> result;
        generate_tilemap_indices_internal(ld, result, [](const life_data& data, auto x, auto y) -> bool {
            return data.living_layer[y * data.width + x];
        });
        return result;
    }

    vector<u32> generate_background_indices(const life_data& ld) noexcept {
        vector<u32> result;
        result.reserve(ld.width * ld.height * 6);
        generate_tilemap_indices_internal(ld, result, [](const life_data& data, auto x, auto y) -> bool {
            E2D_UNUSED(data, x, y);
            return true;
        });
        return result;
    }

    template<typename Ft>
    void generate_tilemap_vertices_internal(const life_data& ld,
                                           vector<v3f>& dest,
                                           Ft&& pred) {
        const f32 hw = map_definition::tile_size;
        const f32 hh = map_definition::tile_size;
        const f32 woff = -hw * f32(ld.width) / 2.f;
        const f32 hoff = -hh * f32(ld.height) / 2.f;

        for(size_t j = 0; j < ld.height; ++j){
            for(size_t i = 0; i < ld.width; ++i) {
                if(!pred(ld, i, j)) {
                    continue;
                }
                dest.emplace_back(woff + hw * (i    ), hoff + hh * (j + 1), 1.f);
                dest.emplace_back(woff + hw * (i    ), hoff + hh * (j    ), 1.f);
                dest.emplace_back(woff + hw * (i + 1), hoff + hh * (j + 1), 1.f);
                dest.emplace_back(woff + hw * (i + 1), hoff + hh * (j    ), 1.f);
            }
        }
    }

    vector<v3f> generate_background_vertices(const life_data& ld) noexcept {
        vector<v3f> result;
        result.reserve(ld.width * ld.height * 4);
        generate_tilemap_vertices_internal(ld, result,
            [](const life_data& data, auto x, auto y) -> bool {
                E2D_UNUSED(data, x, y);
                return true;
            });
        return result;
    }

    vector<v3f> generate_foreground_vertices(const life_data& ld) noexcept {
        vector<v3f> result;
        result.reserve(ld.width * ld.height * 4);
        generate_tilemap_vertices_internal(ld, result,
            [](const life_data& data, auto x, auto y) -> bool {
                return data.living_layer[y * data.width + x];
            });
        return result;
    }

    template<typename Ft>
    void generate_uvs_internal(const life_data& ld,
                               vector<v2f>& dest,
                               Ft&& pred) {
        for(size_t j = 0; j < ld.height; ++j){
            for(size_t i = 0; i < ld.width; ++i) {
                std::tuple<bool, f32, f32> predicateResult = pred(ld, i, j);
                if(!std::get<0>(predicateResult)) {
                    continue;
                }
                f32 woff = std::get<1>(predicateResult);
                f32 hoff = std::get<2>(predicateResult);
                dest.emplace_back(woff      , hoff + .5f);
                dest.emplace_back(woff      , hoff      );
                dest.emplace_back(woff + .5f, hoff + .5f);
                dest.emplace_back(woff + .5f, hoff      );
            }
        }
    }

    vector<v2f> generate_background_uvs(const life_data& ld) noexcept {
        vector<v2f> result;
        result.reserve(ld.width * ld.height * 4);
        generate_uvs_internal(ld, result,
            [](const life_data& data, auto x, auto y) {
                const auto tile_id = data.background_layer[x + y * data.width];
                const auto offs = map_definition::tile_uv_offsets[tile_id];
                return std::make_tuple(true, offs.x, offs.y);
            });
        return result;
    }

    vector<v2f> generate_foreground_uvs(const life_data& ld) noexcept {
        vector<v2f> result;
        result.reserve(ld.width * ld.height * 4);
        generate_uvs_internal(ld, result,
            [](const life_data& data, auto x, auto y) {
                return std::make_tuple(data.living_layer[x + y * data.width], 0.0f, 0.0f);
            });
        return result;
    }

    struct life_system final : public ecs::system {
        void process(ecs::registry& owner) override {
            const auto time = the<engine>().time();
            owner.for_joined_components<life_data>(
                [&time](const ecs::const_entity&, life_data& data){
                    if(time - data.last_generation_time < 0.06f)
                        return;
                    data.last_generation_time = time;

                    #define stride(x, y) (y) * data.width + (x)
                    #define get_living_at(x, y) data.living_layer[stride(x, y)]
                    for(size_t j = 1; j < data.height-1; ++j) {
                        for(size_t i = 1; i < data.width-1; ++i) {
                            uint8_t alive_neighbours = 0;
                            #define test_living(x, y) \
                                if(get_living_at(x, y)) ++alive_neighbours;

                            test_living(i - 1, j - 1)
                            test_living(i    , j - 1)
                            test_living(i + 1, j - 1)

                            test_living(i - 1, j    )
                            test_living(i + 1, j    )

                            test_living(i - 1, j + 1)
                            test_living(i    , j + 1)
                            test_living(i + 1, j + 1)

                            #undef test_living

                            const auto cell_habitated = get_living_at(i, j);
                            const auto romantic_vibe = (alive_neighbours == 3);
                            const auto alone = (alive_neighbours < 2);
                            const auto nothing_to_eat = (alive_neighbours > 3);
                            const auto kill_me_for_mercy = alone || nothing_to_eat;

                            if(!cell_habitated && romantic_vibe) {
                                data.living_buffer[stride(i, j)] = true;
                            } else if(cell_habitated && kill_me_for_mercy) {
                                data.living_buffer[stride(i, j)] = false;
                            } else {
                                data.living_buffer[stride(i, j)] =
                                     data.living_layer[stride(i, j)];
                            }                            
                        }
                    }
                    data.living_layer.swap(data.living_buffer);
                    #undef get_living_at
                    #undef stride
                    data.is_dirty = true;
            });
        }
    };

    struct life_mesh_recalc_system final : public ecs::system {
        void process(ecs::registry& owner) override {
            owner.for_joined_components<life_data>(
                [](const ecs::const_entity&, life_data& data){
                    if(!data.is_dirty)
                        return;
                    data.is_dirty = false;
                    auto fore_e = the<world>().registry().wrap_entity(data.foreground_e_id);
                    model_renderer& rdr = fore_e.get_component<model_renderer>();
                    const model_asset::ptr& model_res = rdr.model();

                    vector<v3f> fore_tangents(0);
                    vector<v3f> fore_bitangents(0);
                    vector<color32> fore_colors(0);

                    mesh fore_mesh;
                    fore_mesh.set_vertices(generate_foreground_vertices(data));
                    fore_mesh.set_indices(0, generate_foreground_indices(data));
                    fore_mesh.set_uvs(0, generate_foreground_uvs(data));
                    fore_mesh.set_colors(0, std::move(fore_colors));
                    fore_mesh.set_tangents(std::move(fore_tangents));
                    fore_mesh.set_bitangents(std::move(fore_bitangents));

                    auto fore_mesh_asset = mesh_asset::create(std::move(fore_mesh));

                    model fore_model;
                    fore_model.set_mesh(fore_mesh_asset);
                    fore_model.regenerate_geometry(the<render>());
                    model_res->fill(std::move(fore_model));
            });
        }
    };

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
            } else if ( k.is_key_just_released(keyboard_key::enter) ) {
                owner.for_joined_components<life_data>(
                    [](const ecs::const_entity&, life_data& data){
                        data.is_dirty = true;
                        for(size_t i = 0; i < data.width * data.height;++i) {
                            data.living_layer[i] =
                               ((i / data.width > 0) &&
                                (i % data.width > 0) &&
                                (i / data.width < data.height - 1) &&
                                (i % data.width < data.width  - 1) &&
                                rand() % 2 == 0);
                        }
                });
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
                        the<window>().virtual_size().cast_to<f32>(), 0.f, 1000.f));
                }
            });
        }
    };

    struct game final : public high_application {
        bool initialize() final {
            return create_scene()
                && create_camera()
                && create_systems();
        }
    private:

        bool create_scene() const {
            life_data life_component;
            srand(unsigned(std::time(nullptr)));
            life_component.living_buffer.fill(false);
            life_component.is_dirty = false;
            for(size_t i = 0; i < map_definition::map_width * map_definition::map_height;++i) {
                if((i / map_definition::map_width > 0) &&
                   (i % map_definition::map_width > 0) &&
                   (i / map_definition::map_width < map_definition::map_height - 1) &&
                   (i % map_definition::map_width < map_definition::map_width  - 1)) {
                    life_component.background_layer[i] =
                        rand() % 2 == 0 ?
                            map_definition::back_tile_1 :
                            map_definition::back_tile_2;
                    life_component.living_layer[i] = rand() % 2 == 0;
                }
                else {
                    life_component.background_layer[i] = map_definition::blocker_tile;
                    life_component.living_layer[i] = false;
                }
            }

            auto tilemap_mat = the<library>().load_asset<material_asset>("material.json");
            if(!tilemap_mat)
                return false;

            vector<v3f> fore_tangents(0);
            vector<v3f> fore_bitangents(0);
            vector<color32> fore_colors(0);

            mesh fore_mesh;
            fore_mesh.set_vertices(generate_foreground_vertices(life_component));
            fore_mesh.set_indices(0, generate_foreground_indices(life_component));
            fore_mesh.set_uvs(0, generate_foreground_uvs(life_component));
            fore_mesh.set_colors(0, std::move(fore_colors));
            fore_mesh.set_tangents(std::move(fore_tangents));
            fore_mesh.set_bitangents(std::move(fore_bitangents));

            auto fore_mesh_asset = mesh_asset::create(std::move(fore_mesh));
            if(!fore_mesh_asset)
                return false;

            model fore_model;
            fore_model.set_mesh(fore_mesh_asset);
            fore_model.regenerate_geometry(the<render>());
            auto fore_model_asset = model_asset::create(std::move(fore_model));
            if(!fore_model_asset)
                return false;

            vector<v3f> back_tangents(0);
            vector<v3f> back_bitangents(0);
            vector<color32> back_colors(0);

            mesh back_mesh;
            back_mesh.set_vertices(generate_background_vertices(life_component));
            back_mesh.set_indices(0, generate_background_indices(life_component));
            back_mesh.set_uvs(0, generate_background_uvs(life_component));
            fore_mesh.set_colors(0, std::move(back_colors));
            back_mesh.set_tangents(std::move(back_tangents));
            back_mesh.set_bitangents(std::move(back_bitangents));

            auto back_mesh_asset = mesh_asset::create(std::move(back_mesh));
            if(!back_mesh_asset)
                return false;

            model back_model;
            back_model.set_mesh(back_mesh_asset);
            back_model.regenerate_geometry(the<render>());
            auto back_model_asset = model_asset::create(std::move(back_model));
            if(!back_model_asset)
                return false;

            life_component.last_generation_time = the<engine>().time();

            ecs::entity scene_e = the<world>().registry().create_entity();
            scene_e.assign_component<scene>(node::create(the<world>()));
            node_iptr scene_r = scene_e.get_component<scene>().root();

            {
                ecs::entity tilemap_e = the<world>().registry().create_entity();
                ecs::entity_filler(tilemap_e)
                    .component<actor>(node::create(tilemap_e, scene_r))
                    .component<life_data>(life_component);

                node_iptr actor_node = tilemap_e.get_component<actor>().node();
                {
                    ecs::entity background_e = the<world>().registry().create_entity();
                    ecs::entity_filler(background_e)
                        .component<actor>(node::create(background_e, actor_node))
                        .component<renderer>(renderer()
                            .materials({tilemap_mat}))
                        .component<model_renderer>(back_model_asset);
                }

                {
                    ecs::entity foreground_e = the<world>().registry().create_entity();
                    ecs::entity_filler(foreground_e)
                        .component<actor>(node::create(foreground_e, actor_node))
                        .component<renderer>(renderer()
                            .materials({tilemap_mat}))
                        .component<model_renderer>(fore_model_asset);
                    tilemap_e.get_component<life_data>().foreground_e_id = foreground_e.id();
                }
            }

            return true;
        }

        bool create_camera() const {
            ecs::entity camera_e = the<world>().registry().create_entity();
            ecs::entity_filler(camera_e)
                .component<camera>(camera()
                    .background({.06f, 0.05f, 0.1f, 1.f}))
                .component<actor>(node::create(camera_e));
            return true;
        }

        bool create_systems() const {
            ecs::registry_filler(the<world>().registry())
                .system<game_system>(world::priority_update)
                .system<life_system>(world::priority_update)
                .system<life_mesh_recalc_system>(world::priority_pre_render)
                .system<camera_system>(world::priority_pre_render);
            return true;
        }
    };
}

SANDBOX_MAIN(game)
