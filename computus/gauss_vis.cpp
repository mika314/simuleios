/*------------gauss_vis.cpp----------------------------------------------------/
*
* Purpose: Simple visualization script for difference scenes for computus video
*          2020
*
*   Notes: Compilation instructions:
*              g++ -L/path/to/GathVL -I/path/to/GathVL -lgathvl -o gauss_vis gauss_vis.cpp `pkg-config --cflags --libs cairo libavformat libavcodec libswresample libswscale libavutil` -Wl,-rpath,/path/to/GathVL
*
*-----------------------------------------------------------------------------*/

#include <iostream>
#include <cmath>
#include <memory>
#include <vector>

#include "include/camera.h"
#include "include/scene.h"

void synodic_scene(camera& cam, scene& world,
                   double end_frame, double timestep, bool aaa_version){

    vec sun_size = {world.size.y*0.09, world.size.y*0.09};
    vec earth_size = {world.size.y*0.045, world.size.y*0.045};
    vec moon_size = {world.size.y*0.018, world.size.y*0.018};

    color sun_clr = {1, 1, 0.5, 1};
    color earth_clr = {0, 1, 1, 1};
    color moon_clr = {0.5, 0.5, 0.5, 1};
    color line_clr = {1, 0, 0, 1};
    color shadow_clr = {1,1,1,1};

    if(aaa_version){
        sun_clr = {1, 0.9, 0, 1};
        earth_clr = {0, 0.75, 0.75, 1};
        shadow_clr = {0,0,0,1};
    }

    double earth_radius = world.size.y*.3;
    double moon_radius = world.size.y*0.1;

    vec earth_offset = vec{sqrt(earth_radius*earth_radius/2),
                           sqrt(earth_radius*earth_radius/2)};

    vec moon_offset = vec{sqrt(moon_radius*moon_radius/2),
                          sqrt(moon_radius*moon_radius/2)};


    vec sun_loc = {world.size.x/2, world.size.y/2};
    vec earth_loc = sun_loc + earth_offset;
    vec moon_loc = earth_loc + moon_offset;

    auto sun = std::make_shared<ellipse>(sun_clr,
                                         sun_loc,
                                         vec{0,0}, 0, true);

    sun->add_animator<vec_animator>(0,20, &sun->size, vec{0,0},
                                    sun_size*1.1);
    sun->add_animator<vec_animator>(20,25, &sun->size, sun_size*1.1,
                                    sun_size);

    std::shared_ptr<ellipse> earth;
    std::shared_ptr<ellipse> moon;
    earth = std::make_shared<ellipse>(earth_clr, earth_loc,
                                     vec{0,0}, 0, true);

    earth->add_animator<vec_animator>(10,30, &earth->size,
                                      vec{0,0}, earth_size*1.1);
    earth->add_animator<vec_animator>(30,35, &earth->size,
                                      earth_size*1.1, earth_size);

    moon = std::make_shared<ellipse>(moon_clr, moon_loc,
                                     vec{0,0}, 0, true);

    moon->add_animator<vec_animator>(20,40, &moon->size,
                                     vec{0,0}, moon_size*1.1);
    moon->add_animator<vec_animator>(40,45, &moon->size,
                                     moon_size*1.1, moon_size);

    std::shared_ptr<arc> earth_shadows;
    std::shared_ptr<arc> moon_shadows;

    earth_shadows = std::make_shared<arc>(shadow_clr,earth_loc,
                                          earth_size*0.95,
                                          vec{0,2*M_PI});
    moon_shadows = std::make_shared<arc>(shadow_clr,moon_loc,
                                         moon_size*0.95,
                                         vec{0,2*M_PI});


    // Creating a theta for the earth that changes with time
    double earth_theta = M_PI/4;
    double moon_theta = M_PI/4;

    double earth_freq = 0.1;
    double moon_freq = earth_freq*365*12/354;

    double gregorian_year_frames = 2*M_PI/earth_freq/timestep;
    double lunar_half_year_frames = gregorian_year_frames*29.53*6/365.2524;

    // creating line connecting sun and moon and adding animator
    auto synodic_line_0 = std::make_shared<line>(line_clr, sun_loc, sun_loc);
    synodic_line_0->add_animator<vec_animator>(lunar_half_year_frames + 60,
                                               lunar_half_year_frames + 90,
                                               &synodic_line_0->end,
                                               sun_loc, moon_loc);

    for (int i = 50; i < lunar_half_year_frames + 51; ++i){
        earth_theta -= earth_freq*timestep;
        moon_theta -= (moon_freq + earth_freq)*timestep;
        vec new_loc = sun_loc + vec(earth_radius*cos(earth_theta),
                                    earth_radius*sin(earth_theta));
        earth->add_animator<vec_animator>(i-1,i, &earth->location,
                                          earth_loc, new_loc);
        earth_loc = new_loc;

        new_loc = earth_loc + vec(moon_radius*cos(moon_theta),
                                  moon_radius*sin(moon_theta));
        moon->add_animator<vec_animator>(i-1,i, &moon->location,
                                         moon_loc, new_loc);
        if (i < lunar_half_year_frames + 50){
            moon_loc = new_loc;
        }
    }

    // Create line connecting sun and new moon
    auto synodic_line_1 = std::make_shared<line>(line_clr, sun_loc, sun_loc);
    synodic_line_1->add_animator<vec_animator>(lunar_half_year_frames + 60,
                                               lunar_half_year_frames + 90,
                                               &synodic_line_1->end,
                                               sun_loc, moon_loc);

    world.add_layer();
    world.add_layer();
    world.add_object(sun, 1);
    world.add_object(earth, 2);
    world.add_object(moon, 2);


    for (int i = 0; i < end_frame; ++i){
        world.update(i);
        if (i == 51){
            world.add_object(earth_shadows, 1);
            world.add_object(moon_shadows, 1);
        }
        if (i == floor(lunar_half_year_frames) + 60){
            world.add_object(synodic_line_0, 2);
            world.add_object(synodic_line_1, 2);
        }
        cam.encode_frame(world);
    }
}

void earth_scene(camera& cam, scene& world, double end_frame, double timestep,
                 bool aaa_version){

    vec sun_size = {world.size.y*0.09, world.size.y*0.09};
    vec earth_size = {world.size.y*0.045, world.size.y*0.045};

    color sun_clr = {1, 1, 0.5, 1};
    color earth_clr = {0, 1, 1, 1};

    if (aaa_version){
        sun_clr = {1, 0.9, 0, 1};
        earth_clr = {0, 0.75, 0.75, 1};
    }

    double earth_radius = world.size.y*.3;

    vec earth_offset = vec{sqrt(earth_radius*earth_radius/2),
                           sqrt(earth_radius*earth_radius/2)};


    vec sun_loc = {world.size.x/2, world.size.y/2};
    vec earth_loc = sun_loc + earth_offset;

    // Don't Judge Me!
    earth_loc = earth_loc;

    auto sun = std::make_shared<ellipse>(sun_clr,
                                         sun_loc,
                                         vec{0,0}, 0, true);

    sun->add_animator<vec_animator>(0,20, &sun->size, vec{0,0},
                                    sun_size*1.1);
    sun->add_animator<vec_animator>(20,25, &sun->size, sun_size*1.1,
                                    sun_size);

    std::shared_ptr<ellipse> earth;
    earth = std::make_shared<ellipse>(earth_clr, earth_loc,
                                     vec{0,0}, 0, true);

    earth->add_animator<vec_animator>(10,30, &earth->size,
                                      vec{0,0}, earth_size*1.1);
    earth->add_animator<vec_animator>(30,35, &earth->size,
                                      earth_size*1.1, earth_size);

    // Creating a theta for the earth that changes with time
    double earth_theta = M_PI/4;

    double earth_freq = 0.1;
    for (int i = 50; i < end_frame; ++i){
        earth_theta -= earth_freq*timestep;
        vec new_loc = sun_loc + vec(earth_radius*cos(earth_theta),
                                    earth_radius*sin(earth_theta));
        earth->add_animator<vec_animator>(i-1,i, &earth->location,
                                          earth_loc, new_loc);
        earth_loc = new_loc;
    }

    world.add_layer();
    world.add_layer();
    world.add_object(sun, 1);
    world.add_object(earth, 2);


    for (int i = 0; i < end_frame; ++i){
        world.update(i);
        cam.encode_frame(world);
    }
}

void moon_scene(camera& cam, scene& world, double end_frame, double timestep,
                bool aaa_version){

    vec sun_size = {world.size.y*0.09, world.size.y*0.09};
    vec earth_size = {world.size.y*0.045, world.size.y*0.045};
    vec moon_size = {world.size.y*0.018, world.size.y*0.018};

    color sun_clr = {1, 1, 0.5, 1};
    color earth_clr = {0, 1, 1, 1};
    color moon_clr = {0.5, 0.5, 0.5, 1};
    color line_clr = {1, 0, 0, 1};
    color shadow_clr = {1,1,1,1};

    if (aaa_version){
        sun_clr = {1, 0.9, 0, 1};
        earth_clr = {0, 0.75, 0.75, 1};
    }

    double earth_radius = world.size.y*0.3;
    double moon_radius = world.size.y*1;

    vec earth_offset = vec{sqrt(earth_radius*earth_radius/2),
                           sqrt(earth_radius*earth_radius/2)};

    vec moon_offset = vec{sqrt(moon_radius*moon_radius/2),
                          sqrt(moon_radius*moon_radius/2)};


    vec sun_loc = {world.size.x/2, world.size.y/2};
    vec earth_loc[3] = {sun_loc + earth_offset, vec(0,0), vec(0,0)};
    vec moon_loc[3] = {earth_loc[0] + moon_offset, vec(0,0), vec(0,0)};

    // Don't Judge Me!
    for (int i = 1; i < 3; ++i){
        earth_loc[i] = earth_loc[0];
        moon_loc[i] = moon_loc[0];
    }

    auto sun = std::make_shared<ellipse>(sun_clr,
                                         sun_loc,
                                         vec{0,0}, 0, true);

    sun->add_animator<vec_animator>(0,20, &sun->size, vec{0,0},
                                    sun_size*1.1);
    sun->add_animator<vec_animator>(20,25, &sun->size, sun_size*1.1,
                                    sun_size);

    std::shared_ptr<ellipse> earth;
    std::shared_ptr<ellipse> moon;
    earth = std::make_shared<ellipse>(earth_clr, earth_loc[0],
                                     vec{0,0}, 0, true);

    earth->add_animator<vec_animator>(10,30, &earth->size,
                                      vec{0,0}, earth_size*1.1);
    earth->add_animator<vec_animator>(30,35, &earth->size,
                                      earth_size*1.1, earth_size);

    moon = std::make_shared<ellipse>(moon_clr, moon_loc[0],
                                     vec{0,0}, 0, true);

    moon->add_animator<vec_animator>(20,40, &moon->size,
                                     vec{0,0}, moon_size*1.1);
    moon->add_animator<vec_animator>(40,45, &moon->size,
                                     moon_size*1.1, moon_size);


    // Creating a theta for the earth that changes with time
    double earth_theta[2] = {M_PI/4, 0};
    double moon_theta[2] = {M_PI/4, 0};

    double earth_freq = 0.1;
    double moon_freq = earth_freq*365*12/354;

    double gregorian_year_frames = 2*M_PI/earth_freq/timestep;
    double lunar_year_frames = gregorian_year_frames*29.53*12/365.2524;
    int indices = 2;
    for (int i = 50; i < gregorian_year_frames + 51; ++i){
        moon_theta[0] -= (moon_freq + earth_freq)*timestep;
        for (int j = 0; j < indices; ++j){
            vec new_loc = earth_loc[0] + vec(moon_radius*cos(moon_theta[0]),
                                             moon_radius*sin(moon_theta[0]));
            moon->add_animator<vec_animator>(i-1,i, &moon->location,
                                             moon_loc[0], new_loc);
            moon_loc[j] = new_loc;
        }
    }
    moon_theta[1] = fmod(moon_theta[0] + (moon_freq+earth_freq)*timestep,
                         2*M_PI);
    moon_theta[0] = M_PI/4 + (moon_freq+earth_freq)*timestep;

    world.add_layer();
    world.add_layer();
    world.add_object(sun, 1);
    world.add_object(earth, 2);
    world.add_object(moon, 2);


    for (int i = 0; i < end_frame; ++i){
        world.update(i);
        cam.encode_frame(world);
    }
}

void orbit_scene(camera& cam, scene& world, double end_frame, double timestep,
                 bool aaa_version){

    vec sun_size = {world.size.y*0.09, world.size.y*0.09};
    vec earth_size = {world.size.y*0.045, world.size.y*0.045};
    vec moon_size = {world.size.y*0.018, world.size.y*0.018};

    color sun_clr = {1, 1, 0.5, 1};
    color earth_clr = {0, 1, 1, 1};
    color moon_clr = {0.5, 0.5, 0.5, 1};
    color line_clr = {1, 0, 0, 1};
    color shadow_clr = {1,1,1,1};

    if (aaa_version){
        sun_clr = {1, 0.9, 0, 1};
        earth_clr = {0, 0.75, 0.75, 1};
        shadow_clr = {0, 0, 0, 1};
    }

    double earth_radius = world.size.y*.3;
    double moon_radius = world.size.y*.1;

    vec earth_offset = vec{sqrt(earth_radius*earth_radius/2),
                           sqrt(earth_radius*earth_radius/2)};

    vec moon_offset = vec{sqrt(moon_radius*moon_radius/2),
                          sqrt(moon_radius*moon_radius/2)};


    vec sun_loc = {world.size.x/2, world.size.y/2};
    vec earth_loc[3] = {sun_loc + earth_offset, vec(0,0), vec(0,0)};
    vec moon_loc[3] = {earth_loc[0] + moon_offset, vec(0,0), vec(0,0)};

    // Don't Judge Me!
    for (int i = 1; i < 3; ++i){
        earth_loc[i] = earth_loc[0];
        moon_loc[i] = moon_loc[0];
    }

    auto sun = std::make_shared<ellipse>(sun_clr,
                                         sun_loc,
                                         vec{0,0}, 0, true);

    sun->add_animator<vec_animator>(0,20, &sun->size, vec{0,0},
                                    sun_size*1.1);
    sun->add_animator<vec_animator>(20,25, &sun->size, sun_size*1.1,
                                    sun_size);

    std::shared_ptr<ellipse> earth;
    std::shared_ptr<ellipse> moon;
    earth = std::make_shared<ellipse>(earth_clr, earth_loc[0],
                                     vec{0,0}, 0, true);

    earth->add_animator<vec_animator>(10,30, &earth->size,
                                      vec{0,0}, earth_size*1.1);
    earth->add_animator<vec_animator>(30,35, &earth->size,
                                      earth_size*1.1, earth_size);

    moon = std::make_shared<ellipse>(moon_clr, moon_loc[0],
                                     vec{0,0}, 0, true);

    moon->add_animator<vec_animator>(20,40, &moon->size,
                                     vec{0,0}, moon_size*1.1);
    moon->add_animator<vec_animator>(40,45, &moon->size,
                                     moon_size*1.1, moon_size);


    // Creating a theta for the earth that changes with time
    double earth_theta[2] = {M_PI/4, 0};
    double moon_theta[2] = {M_PI/4, 0};

    double earth_freq = 0.1;
    double moon_freq = earth_freq*365*12/354;

    double gregorian_year_frames = 2*M_PI/earth_freq/timestep;
    double lunar_year_frames = gregorian_year_frames*29.53*12/365.2524;
    int indices = 2;
    for (int i = 50; i < gregorian_year_frames + 51; ++i){
        //if (i > lunar_year_frames + 32){
        if (i == floor(lunar_year_frames) + 51){
            indices = 1;
            earth_theta[1] = fmod(earth_theta[0] + earth_freq*timestep, 2*M_PI);
        }
        earth_theta[0] -= earth_freq*timestep;
        moon_theta[0] -= (moon_freq + earth_freq)*timestep;
        for (int j = 0; j < indices; ++j){
            vec new_loc = sun_loc + vec(earth_radius*cos(earth_theta[0]),
                                        earth_radius*sin(earth_theta[0]));
            earth->add_animator<vec_animator>(i-1,i, &earth->location,
                                              earth_loc[0], new_loc);
            earth_loc[j] = new_loc;

            new_loc = earth_loc[0] + vec(moon_radius*cos(moon_theta[0]),
                                         moon_radius*sin(moon_theta[0]));
            moon->add_animator<vec_animator>(i-1,i, &moon->location,
                                             moon_loc[0], new_loc);
            moon_loc[j] = new_loc;
        }
    }
    moon_theta[1] = fmod(moon_theta[0] + (moon_freq+earth_freq)*timestep,
                         2*M_PI);
    moon_theta[0] = M_PI/4 + (moon_freq+earth_freq)*timestep;

    // Drawing the line for the Earths
    auto earth_arc = std::make_shared<arc>(line_clr, sun_loc,
                                           vec(earth_radius,earth_radius),
                                           vec{0,0});
    earth_arc->add_animator<vec_animator>(50+gregorian_year_frames,
                                          80+gregorian_year_frames,
                                          &earth_arc->angles,
                                          vec{earth_theta[0], earth_theta[0]},
                                          vec{earth_theta[0], earth_theta[1]});

    // Drawing the line for the Earths
    auto moon_arc = std::make_shared<arc>(line_clr, earth_loc[0],
                                          vec(moon_radius,moon_radius),
                                          vec{0,0});
    moon_arc->add_animator<vec_animator>(90+gregorian_year_frames,
                                         120+gregorian_year_frames,
                                         &moon_arc->angles,
                                         vec{moon_theta[1], moon_theta[1]},
                                         vec{moon_theta[1], moon_theta[0]});

    std::shared_ptr<arc> earth_shadows[2];
    std::shared_ptr<arc> moon_shadows[2];

    for (int i = 0; i < 2; ++i){
        earth_shadows[i] = std::make_shared<arc>(shadow_clr,earth_loc[i+1],
                                                 earth_size*0.95,
                                                 vec{0,2*M_PI});
        moon_shadows[i] = std::make_shared<arc>(shadow_clr,moon_loc[i+1],
                                                moon_size*0.95,
                                                vec{0,2*M_PI});
    }

    world.add_layer();
    world.add_layer();
    world.add_object(sun, 1);
    world.add_object(earth, 2);
    world.add_object(moon, 2);


    for (int i = 0; i < end_frame; ++i){
        world.update(i);
        if (i == 51){
            world.add_object(earth_shadows[1], 1);
            world.add_object(moon_shadows[1], 1);
        }
        if (i == floor(lunar_year_frames) + 51){
            world.add_object(earth_shadows[0], 1);
            world.add_object(moon_shadows[0], 1);
        }
        if (i == 50+floor(gregorian_year_frames)){
            world.add_object(earth_arc, 2);
        }
        if (i == 90+floor(gregorian_year_frames)){
            world.add_object(moon_arc, 2);
        }
        cam.encode_frame(world);
    }
}

int main() {
    camera cam(vec{1280, 720});
    scene world = scene({1280, 720}, {0, 0, 0, 1});
    //world.bg_clr = {1,1,1,1};

    //cam.add_encoder<png_encoder>();
    cam.add_encoder<video_encoder>("/tmp/video.mp4", cam.size, 60);

    orbit_scene(cam, world, 2000, 0.05, false);
    //earth_scene(cam, world, 2000, 0.05, false);
    //moon_scene(cam, world, 2000, 0.05, false);
    //synodic_scene(cam, world, 4000, 0.01, true);

    cam.clear_encoders();

    return 0;
}
