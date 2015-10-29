#-------------demon.py---------------------------------------------------------#
#
#             demon.py
# 
# Purpose: Visualize the maxwell's demon code from earlier on stream!
#
#------------------------------------------------------------------------------#

import bpy
import numpy as np

#------------------------------------------------------------------------------#
# MAIN
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# SUBROUTINES
#------------------------------------------------------------------------------#

# goes through all the data! Woo!
def parse_data(num_part):
    array = [[]*8]*(num_part)
    i = 0
    offset = 0
    print("importing data from file")
    input = "file.dat"
    with open(input, 'r') as data:
        for line in data:
            if line != '\n':
                temp = [float(s) for s in line.split()]
                array[(i) % num_part] = temp
                i += 1
            if i % num_part == 0 and i != 0:
                place_spheres(array, num_part, i)
                #scene.update()

# Creates sphere material
def create_new_material (passedName,passedcolor):
    tempMat = bpy.data.materials.new(passedName)
    if tempMat != None:
        tempMat.diffuse_color = passedcolor
        tempMat.diffuse_shader = 'LAMBERT'
        tempMat.diffuse_intensity = 1.0
        tempMat.specular_color = (0.9,0.9,0.9)
        tempMat.specular_shader = 'COOKTORR'
        tempMat.specular_intensity = 0.5
        tempMat.use_transparency=False
        tempMat.alpha = 0.5
        tempMat.ambient = 0.3
        tempMat.emit = 0.2
    return tempMat

# places new sphere at given location
def new_sphere(diam, x, y, z, r, g, b, id):
    temp_sphere = bpy.ops.mesh.primitive_uv_sphere_add(segments = 64, 
                                                       ring_count = 32,
                                                       size = diam,
                                                       location = (x, y, z),
                                                       rotation = (0, 0, 0))
    ob = bpy.context.active_object
    ob.name = str(id)
    me = ob.data
    color = (r, g, b)
    mat = create_new_material("myNewMaterial",color)
    me.materials.append(mat)
    return temp_sphere

# places sphere duplicates around for fun!
def place_duplicates(x, y, z, id, ob = None):
    if not ob:
        ob = bpy.context.active_object
    obs = []
    sce = bpy.context.scene
        
    copy = ob.copy()
    copy.location = x,y,z
    copy.data = copy.data.copy()
    copy.name = str(id)
    obs.append(copy)
    
    for ob in obs:
        sce.objects.link(ob)
    
    #sce.update()

# function to place spheres in blender
def place_spheres(array, num_part, i):
    diam = 0.1

    #print(array)

    if i == num_part:
        for i in range(0, num_part):
            if i == 0:
                new_sphere(diam, array[i][0], array[i][1], array[i][2], 0, 0, 1,
                           array[i][7])
            else:
                place_duplicates(array[i][0], array[i][1], array[i][2], 
                                 array[i][7])
    else:
        move_spheres(array, num_part, (i / num_part) * 1)

# Function to moves spheres that are already there.
# Not currently working!
def move_spheres(array, num_part, frame):
    for i in range(num_part):
        bpy.context.scene.frame_set(frame)  
        #print(str(array[i][7]))
        ob = bpy.context.scene.objects[str(array[i][7])]
        ob.location = (array[i][0], array[i][1], array[i][2])
        ob.keyframe_insert(data_path="location", index=-1)

# Creates the cage material
def create_cage (passedName):
    cageMat = bpy.data.materials.new(passedName)
    cageMat.type = 'WIRE'
    cageMat.diffuse_color = (1,1,1)
    cageMat.diffuse_shader = 'FRESNEL'
    cageMat.diffuse_intensity = 1
    cageMat.specular_color = (1,1,1)
    cageMat.use_diffuse_ramp = True
    ramp = cageMat.diffuse_ramp
    #(pt_location_on_ramp, (r,g,b,dens_at_pt))
    values = [(0.0, (1,1,1,1)), (1.0, (1,1,1,1))]
    for n,value in enumerate(values):
        ramp.elements.new((n+1)*0.2)
        elt = ramp.elements[n]
        (pos, color) = value
        elt.position = pos
        elt.color = color
    cageMat.diffuse_ramp_input = 'RESULT'
    return cageMat

# Creates cage at location
def cage_set(Box_length, sign):
    ccube = bpy.ops.mesh.primitive_cube_add(location=(sign * Box_length / 2,Box_length 
/ 2, Box_length / 2), radius = Box_length / 2)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.object.mode_set(mode='OBJECT')
    ob = bpy.context.object
    me = ob.data
    mat = create_cage('MaterialCage')
    me.materials.append(mat)
    return ccube

# Removes objects in scene
def remove_obj( scene ):
    for ob in scene.objects: 
        if ob.name !='Camera':
            scene.objects.unlink( ob )

#defining our scene
def def_scene(box_length, bgcolor):

    # Camera stuff
    #x_cam = 2.2
    #y_cam = 2.75
    #z_cam = 1.43
    #r_camx = 70
    #r_camy = 0
    #r_camz = 145

    x_cam = 0
    y_cam = 0.5
    z_cam = 3
    r_camx = 0
    r_camy = 0
    r_camz = 0

    scene.camera.location.x = box_length * x_cam
    scene.camera.location.y = box_length * y_cam
    scene.camera.location.z = box_length * z_cam

    scene.camera.rotation_mode = 'XYZ'
    scene.camera.rotation_euler[0] = (np.pi/180.0) * r_camx
    scene.camera.rotation_euler[1] = (np.pi/180.0) * r_camy
    scene.camera.rotation_euler[2] = (np.pi/180.0) * r_camz

    # Sets field of view
    scene.camera.data.angle = 50*(np.pi/180.0)

    # Scene resolution
    scene.render.resolution_x = 1366*2
    scene.render.resolution_y = 768*2

    # Remove lighting (for now)
    remove_obj( scene )

    # sets background to be black
    bpy.data.worlds['World'].horizon_color = (0,0,0)

    return scene

# Renders movie
def render_movie(scene):
    scene = bpy.context.scene
    bpy.data.scenes[0].render.image_settings.file_format="PNG"
    #bpy.data.scenes[0].render.filepath = "images/image%.5d" %iteration
    bpy.ops.render.render( write_still=True )
    print("rendering movie")
    scene.sequence_editor_create()
    bpy.data.scenes["Scene"].render.fps = 30
    bpy.data.scenes["Scene"].render.image_settings.file_format = 'FFMPEG'
    #bpy.data.scenes["Scene"].render.ffmpeg.video_bitrate = 24300
    bpy.data.scenes["Scene"].render.ffmpeg.format = 'MPEG4'
    bpy.data.scenes["Scene"].render.ffmpeg.audio_codec = 'NONE'
    bpy.data.scenes["Scene"].render.ffmpeg.minrate = 0
    bpy.data.scenes["Scene"].render.ffmpeg.maxrate = 30000
    bpy.data.scenes["Scene"].render.ffmpeg.codec = 'MPEG4'
    bpy.data.scenes["Scene"].render.filepath = 'out.mp4'
    bpy.data.scenes["Scene"].render.use_file_extension = False
    bpy.ops.render.render( animation=True ) 

scene = bpy.context.scene
scene = def_scene(10,scene)
remove_obj(scene)
parse_data(5)
cage_set(10, 1)
cage_set(10, -1)
scene.update()
render_movie(scene)
#print (array)
