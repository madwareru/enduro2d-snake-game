#ifndef SHARED_HPP
#define SHARED_HPP

#define SANDBOX_MAIN(APP_CLASS)                                                      \
    int e2d_main(int argc, char *argv[]) {                                           \
        const auto starter_params = starter::parameters(                             \
            engine::parameters(SANDBOX_NAME, "enduro2d")                             \
                .timer_params(engine::timer_parameters()                             \
                    .maximal_framerate(100)))                                        \
                .library_root(url{"resources://" SANDBOX_DATA_DIR});                 \
                                                                                     \
        modules::initialize<starter>(argc, argv, starter_params).start<APP_CLASS>(); \
        modules::shutdown<starter>();                                                \
        return 0;                                                                    \
    }

#endif // SHARED_HPP
