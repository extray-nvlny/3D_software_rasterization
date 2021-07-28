typedef struct Camera
{
    // NOTE(shvayko): Camera world position
    v3 p;
    
    v3 direction;
    
    
    // NOTE n vector is analogous to the z-axis(points toward the target p from camera p).
    // v called "up" vector begins from (0,1,0) but this value only temporary and its 
    // needed for counstructing right vector(u vector) and after that new up vector 
    // caclulated.
    // u called "right vector" and it is calculated by cross product of v and n
    // NOTE(shvayko): look at(target)
    v3 u,v,n; // Must be normalized at some point
    
    v3 target;
    
    f32 view_dist_h;
    f32 view_dist_v;
    
    f32 field_of_view;
    
    f32 near_z;
    f32 far_z;
    
    f32 aspect_ratio;
    
    f32 viewport_center_x;
    f32 viewport_center_y;
    
    m4x4 camera_matrix;
}Camera;



void
camera_init(Camera *camera, v3 camera_p, v3 cam_dir, f32 near_z, f32 far_z,
            f32 fov, f32 viewport_width, f32 viewport_height,
            v3 cam_target)
{
    // NOTE View reference point  position in space
    
    camera->direction = cam_dir;
    camera->target = cam_target;
    camera->near_z = near_z;
    camera->far_z = far_z;
    camera->field_of_view = fov;
    camera->u = v3f(1.0f,0.0f,0.0f);
    camera->v = v3f(0.0f,1.0f,0.0f);
    camera->n = v3f(0.0f,0.0f,1.0f); // LHS 
    camera->p = camera_p;
    camera->aspect_ratio = viewport_width / viewport_height;
    
    camera->viewport_center_x = (viewport_width - 1) / 2.0f;
    camera->viewport_center_y = (viewport_height - 1) / 2.0f;
    
    camera->target = cam_target;
    f32 viewplane_width = 2.0f;
    f32 tangent_fov = tan(DEG_TO_RAD(fov/2.0f));
    f32 camera_view_dist = (0.5f*viewplane_width)*tangent_fov;
    
}

void
build_camera_matrix_uvn(Camera *camera)
{
    m4x4 inverse_translation = 
    {
        1.0f,0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        camera->p.x,-camera->p.y,-camera->p.z,1.0f,
    };
    
    f32 phi = camera->direction.x; // Elevation
    f32 theta = camera->direction.y; // Heading
    
    f32 sin_phi = sinf(phi);
    f32 cos_phi = cosf(phi);
    
    f32 sin_theta = sinf(theta);
    f32 cos_theta = cosf(theta);
    
    camera->target.x = -1.0f*sin_phi*sin_theta;
    camera->target.y = 1.0f*cos_phi;
    camera->target.z = 1.0f*sin_phi*cos_theta;
    
    camera->n = normalize_v3(subtract_v3v3(camera->target, camera->p));
    camera->v = v3f(0.0,1.0f,0.0f);
    camera->u = normalize_v3(cross_product_3(camera->v,camera->n));
    camera->v = normalize_v3(cross_product_3(camera->n,camera->u));
    
    m4x4 uvn =
    {
        {
            camera->u.x,camera->v.x,camera->n.x,0.0f,
            camera->u.y,camera->v.y,camera->n.y,0.0f,
            camera->u.z,camera->v.z,camera->n.z,0.0f,
            0.0f,0.0f,0.0f,1.0f,
        }
    };
    
    camera->camera_matrix = mul_m4x4m4x4(inverse_translation, uvn);
}

void
build_camera_matrix_euler(Camera *camera, s32 camera_rotation_seq)
{
    // TODO(shvayko): Create!
}