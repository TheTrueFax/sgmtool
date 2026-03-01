#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h> // For ceil()

#define VERSION "0.1.2"
#define SGM_VERSION 3
#define SGM_MAGIC_NUMBER 352658064

#define uint8 uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define float32 float

// TODO: fix the indices being absolute bum, random faces linking to random verts for no reason
// (around line 900)
// Also fix memory freeing on line 958

// -w [file] input obj
// -o [file] output sgm
// -h or --help is help menu
// -v or --version is version
// -d or --debug is debug (print extra info)

typedef struct ptrlib;
typedef struct {
    void* data[20];
    int index;
    ptrlib* next;
} ptrlib;

void amalloc(size_t size) {

}

void afreeall() {

}



int g_debug = 0;

int print_help() {
    printf("Sgmtool is a CLI tool to convert OBJ files (or different file types) into Uberpixel SGM files, used in the Rayne game engine.\n\nSgmtool help menu:\n\n-h or --help       Show this menu\n-v or --version    Print version number\n-d or --debug      Enable debug info (more entertaining really)\n\nUsage:\n-o [file name]     File to output to, creates a new file if none exists\n-w [file name]     Use a Wavefront .obj file for the conversion\n");
    return 0;
}
int print_version() {
    printf("%s\n",VERSION);
    return 0;
}

void err(const char* text) {
    printf("\x1b[1;31mError: %s\x1b[m\n",text);
}

// Define vector data types (rarely used in favor of sgmable)

struct vec4 {
    float x;
    float y;
    float z;
};
struct vec3 {
    float x;
    float y;
    float z;
};
struct vec2 {
    float x;
    float y;
};

// Define SGM file as a struct, define the dependencies first

struct sgmtexture {
    uint8 texture_type_hint;
    uint16 filename_length;
    char* filename[];
};

struct sgmuvset {
    uint8 number_of_textures;
    struct sgmtexture *textures;
};

struct sgmcolor {
    uint8 color_type_hint;

    float32 r;
    float32 g;
    float32 b;
    float32 a;
};

struct sgmmaterial {
    uint8 material_id;

    uint8 number_of_uv_sets;
    struct sgmuvset *uvs;

    uint8 number_of_colors;
    struct sgmcolor *colors;
};

struct sgmuvpos {
    float u;
    float v;
};

struct sgmvertex {
    float position_x;
    float position_y;
    float position_z;

    float normal_x;
    float normal_y;
    float normal_z;
    
    struct sgmuvpos *uvs; // One item for each layer

    uint8 has_color;
    uint8 has_tangents;
    uint8 has_bones;

    float color_r;
    float color_g;
    float color_b;
    float color_a;
    
    float tangents_x;
    float tangents_y;
    float tangents_z;
    float tangents_w;
    
    float weights_x;
    float weights_y;
    float weights_z;
    float weights_w;
    
    float bones_x;
    float bones_y;
    float bones_z;
    float bones_w;
};

struct sgmmesh {
    uint8 mesh_id;
    uint8 used_materials_id;
    uint32 number_of_vertices;
    uint8 texcoord_count;
    uint8 color_channel_count;
    uint8 has_tangents;
    uint8 has_bones;

    struct sgmvertex *vertex_data;

    uint32 number_of_indices;
    uint8 index_size;
    uint16 *indices_2;
    uint32 *indices_4;
};

// Root file object, start here for discovery

struct sgmfile {
    uint32 magic_number;
    uint8 version;

    uint8 number_of_materials;
    struct sgmmaterial *materials;

    uint8 number_of_meshes;
    struct sgmmesh *meshes;

    uint8 has_animation;
    uint16 animfilename_length;
    char* animfilename[];
};
    
int serialize_sgmfile(const char* filename, struct sgmfile sgm, int exptextures, int exptangents, int expanimations, int hasbones) {
    // Write sgmfile struct into a file, then free sgmfile memory

    if (g_debug)
        printf("Starting serialization\n");

    FILE* fileptr;

    fileptr = fopen(filename, "wb");
    if (fileptr==NULL) {
        err("Error opening output file for writing, check if you have permissions for the file.\n");
        return 1;
    }

    if (g_debug)
        printf("|-- Writing headers\n");


    // im so sorry i have to hardcode everything

    fwrite((void*)&sgm.magic_number, 1, 4, fileptr);
    fwrite((void*)&sgm.version, 1, 1, fileptr);

    fwrite((void*)&sgm.number_of_materials, 1, 1, fileptr);

    if (g_debug)
        printf("|-- Writing %i material(s)\n",sgm.number_of_materials);
    
    for (int i=0;i<sgm.number_of_materials;i++) { // for every material
        if (g_debug)
            printf("| Writing material headers\n");
        fwrite((void*)&sgm.materials[i].material_id, 1, 1, fileptr);
        fwrite((void*)&sgm.materials[i].number_of_uv_sets, 1, 1, fileptr);
        if (sgm.materials[i].number_of_uv_sets>0) {
            if (g_debug)
                printf("| |-- Writing uv set(s)\n");
        }
        for (int x=0;x<sgm.materials[i].number_of_uv_sets;x++) { // for every uv set
            fwrite((void*)&sgm.materials[i].uvs[x].number_of_textures, 1, 1, fileptr);
            for (int z=0;z<sgm.materials[i].uvs[x].number_of_textures;z++) { // for every texture
                fwrite((void*)&sgm.materials[i].uvs[x].textures[z].texture_type_hint, 1, 1, fileptr);
                fwrite((void*)&sgm.materials[i].uvs[x].textures[z].filename_length, 1, 2, fileptr);
                fwrite((void*)sgm.materials[i].uvs[x].textures[z].filename, 1, sgm.materials[i].uvs[x].textures[z].filename_length, fileptr);
                free(sgm.materials[i].uvs[x].textures[z].filename);
            }
            if (sgm.materials[i].uvs[x].number_of_textures>0){
                free(sgm.materials[i].uvs[x].textures);
            }
        }
        /*if (sgm.materials[i].number_of_uv_sets!=0){
            free(sgm.materials[i].uvs);
        }*/
        if (g_debug)
            printf("| |-- Writing material color data\n");
        fwrite((void*)&sgm.materials[i].number_of_colors, 1, 1, fileptr);
        for (int x=0;x<sgm.materials[i].number_of_colors;x++) { // for every color
            fwrite((void*)&sgm.materials[i].colors[x].color_type_hint, 1, 1, fileptr);
            fwrite((void*)&sgm.materials[i].colors[x].r, 1, 4, fileptr);
            fwrite((void*)&sgm.materials[i].colors[x].g, 1, 4, fileptr);
            fwrite((void*)&sgm.materials[i].colors[x].b, 1, 4, fileptr);
            fwrite((void*)&sgm.materials[i].colors[x].a, 1, 4, fileptr);
        }
        if (sgm.materials[i].number_of_colors>0) {
            free(sgm.materials[i].colors);
        }
    }

    if (g_debug)
        printf("|-- Writing %i mesh(es)\n",sgm.number_of_meshes);

    fwrite((void*)&sgm.number_of_meshes, 1, 1, fileptr);
    for (int i=0;i<sgm.number_of_meshes;i++) { // for every mesh

        if (g_debug)
            printf("| Writing mesh headers\n");
        fwrite((void*)&sgm.meshes[i].mesh_id, 1, 1, fileptr);
        fwrite((void*)&sgm.meshes[i].used_materials_id, 1, 1, fileptr);
        fwrite((void*)&sgm.meshes[i].number_of_vertices, 1, 4, fileptr);
        fwrite((void*)&sgm.meshes[i].texcoord_count, 1, 1, fileptr);
        fwrite((void*)&sgm.meshes[i].color_channel_count, 1, 1, fileptr);
        fwrite((void*)&sgm.meshes[i].has_tangents, 1, 1, fileptr);
        fwrite((void*)&sgm.meshes[i].has_bones, 1, 1, fileptr);

        if (g_debug)
            printf("| |-- Writing interleaved verticies\n");

        for (int x=0;x<sgm.meshes[i].number_of_vertices;x++) { // for every vertex
            fwrite((void*)&sgm.meshes[i].vertex_data[x].position_x, 1, 4, fileptr);
            fwrite((void*)&sgm.meshes[i].vertex_data[x].position_y, 1, 4, fileptr);
            fwrite((void*)&sgm.meshes[i].vertex_data[x].position_z, 1, 4, fileptr);
            //printf("Vertex num %i, x: %f, y: %f, z: %f\n",x,sgm.meshes[i].vertex_data[x].position_x,sgm.meshes[i].vertex_data[x].position_y,sgm.meshes[i].vertex_data[x].position_z);
            
            fwrite((void*)&sgm.meshes[i].vertex_data[x].normal_x, 1, 4, fileptr);
            fwrite((void*)&sgm.meshes[i].vertex_data[x].normal_y, 1, 4, fileptr);
            fwrite((void*)&sgm.meshes[i].vertex_data[x].normal_z, 1, 4, fileptr);
            
            for (uint8 v=0;v<sgm.materials[sgm.meshes[i].used_materials_id].number_of_uv_sets;v++) {
                fwrite((void*)&sgm.meshes[i].vertex_data[x].uvs[v].u, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].uvs[v].v, 1, 4, fileptr);
            }

            if (sgm.meshes[i].vertex_data[0].has_color!=0) { // color exists
                fwrite((void*)&sgm.meshes[i].vertex_data[x].color_r, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].color_g, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].color_b, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].color_a, 1, 4, fileptr);
            }
            
            if (exptangents!=0) {
                fwrite((void*)&sgm.meshes[i].vertex_data[x].tangents_x, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].tangents_y, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].tangents_z, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].tangents_w, 1, 4, fileptr);
            }
            
            if (hasbones!=0) {
                fwrite((void*)&sgm.meshes[i].vertex_data[x].weights_x, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].weights_y, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].weights_z, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].weights_w, 1, 4, fileptr);

                fwrite((void*)&sgm.meshes[i].vertex_data[x].bones_x, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].bones_y, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].bones_z, 1, 4, fileptr);
                fwrite((void*)&sgm.meshes[i].vertex_data[x].bones_w, 1, 4, fileptr);
            }
        }

        if (g_debug)
            printf("| |-- Writing %i indice(s)\n",sgm.meshes[i].number_of_indices);

        fwrite((void*)&sgm.meshes[i].number_of_indices, 1, 4, fileptr);
        fwrite((void*)&sgm.meshes[i].index_size, 1, 1, fileptr);
        int index_size = sgm.meshes[i].index_size;
        for (int x=0;x<sgm.meshes[i].number_of_indices;x++) { // for each index
            if (index_size==2) {
                fwrite((void*)&sgm.meshes[i].indices_2[x], 1, 2, fileptr);
            } else if (index_size==4) {
                fwrite((void*)&sgm.meshes[i].indices_4[x], 1, 4, fileptr);
            }
        }
        if (sgm.meshes[i].number_of_indices>0){
            if (index_size==2){
                free(sgm.meshes[i].indices_2);
            } else {
                free(sgm.meshes[i].indices_4);
            }
        }
        if (g_debug)
            printf("| |-- Finished writing mesh\n");
    }
    for (int i=0;i<sgm.number_of_meshes;i++) {
        if (sgm.materials[sgm.meshes[i].used_materials_id].uvs!=NULL) {
            free(sgm.materials[sgm.meshes[i].used_materials_id].uvs);
        }
    }
    
    if (sgm.number_of_materials>0) {
        free(sgm.materials);
    }
    if (sgm.number_of_meshes>0){
        free(sgm.meshes);
    }

    if (g_debug)
        printf("|-- Writing animation data\n");

    // animations
    fwrite((void*)&sgm.has_animation, 1, 1, fileptr);
    fwrite((void*)&sgm.animfilename_length, 1, 2, fileptr);
    if (sgm.animfilename_length!=0){
        fwrite((void*)&sgm.animfilename, 1, sgm.animfilename_length, fileptr);
    }

    fclose(fileptr);

    if (g_debug)
        printf("Done!\n");

    return 0;
}

int count_chars(const char* string, const char* token) {
    int amount = 0;
    for (int i=0;i<strlen(string);i++) {
        if (string[i]==*token) {
            amount++;
        }
    }
    return amount;
}

int str_index(const char* array[], int array_length, const char* string) {
    for (int i=0;i<array_length;i++) {
        if (strcmp(array[i],string)==0) {
            return i+1;
        }
    }
    return 0;
}

int atoi_cool(const char* string) {
    if (strlen(string)<2) {
        return string[0]-'0';
    } else {
        return atoi(string);
    }
}

void sgm_fromwavefront(const char* filename, struct sgmfile* sgm) {
    if (g_debug)
        printf("Opening wavefront file\n");
    
    // write const headers
    sgm->magic_number = SGM_MAGIC_NUMBER;
    sgm->version = SGM_VERSION;

    FILE* fileptr;

    fileptr = fopen(filename, "r"); // no rb because obj is plaintext
    if (fileptr==NULL) {
        err("Can't open obj file for reading.");
        return;
    }

    char line[1000];

    char* septoken = " ";
    char* faceseptoken = "/";

    // verts and faces structure
    struct linqvert;
    struct linqvert {
        int uto;
        float points[30]; // 10 verts
        struct linqvert* next;
        struct linqvert* last;
    };

    struct facedata {
        int vert_id;
        int uv_id;
        int norm_id;
    };

    int fsmooth = 0;

    struct linqface;
    struct linqface {
        int material;
        int vert_count;
        int is_smooth;
        struct facedata* data;
        struct linqface* next;
        struct linqface* last;
    };

    int mat_count = 0;
    int current_mat = 0;
    char* matnames[256];

    struct linqvert* getitem(int index, struct linqvert* starting) {
        struct linqvert* current = starting;
        for (int i=0;i<index;i++) {
            current = current->next;
        }
        return current;
    }
    struct linqface* getfaceitem(int index, struct linqface* starting) {
        struct linqface* current = starting;
        for (int i=0;i<index;i++) {
            current = current->next;
        }
        return current;
    }
    
    int vert_count = 0;
    int vert_uv_count = 0;
    int vert_norm_count = 0;

    int face_count = 0;

    int has_ngons = 0;
    int has_norms = 0;
    int has_uvs = 0;

    // faces to link
    struct linqface faces;
    struct linqface* lastface = &faces;

    // defined uv coords for verts
    struct linqvert uvverts;
    struct linqvert* lastuvvert = &uvverts;

    // defined vert normals
    struct linqvert normverts;
    struct linqvert* lastnormvert = &normverts;

    // defined vert positions
    struct linqvert verts;
    struct linqvert* lastvert = &verts;

    if (g_debug)
        printf("Parsing file data\n");

    while (fgets(line, sizeof(line), fileptr)) {
        // split line by spaces and remove \n
        char** splittedlist;

        if (line[0]=='#') {
            continue;
        }
        if (strlen(line)<2) {
            continue;
        }

        int amount_of_phrases = count_chars(line,septoken)+1;

        if (amount_of_phrases<2) {
            continue;
        }

        splittedlist = malloc(amount_of_phrases * sizeof(char*));

        splittedlist[0] = strtok(line, septoken);
        for (int i=1;i<amount_of_phrases;i++) {
            splittedlist[i] = strtok(NULL, septoken);
        }
        // set return char to 0
        splittedlist[amount_of_phrases-1][strlen(splittedlist[amount_of_phrases-1])-1]=(char)0x00;

        /*for (int i=0;i<amount_of_phrases;i++) {
            printf("%s\n",splittedlist[i]);
        }*/

        if (strcmp(splittedlist[0],"s")==0) {
            if (strcmp(splittedlist[1],"on")==0) {
                fsmooth=1;
            } else if (strcmp(splittedlist[1],"off")==0) {
                fsmooth=0;
            } else {
                printf("\x1b[1;33mWarning: Invalid ss option in OBJ file\x1b[m");
            }
        }

        if (strcmp(splittedlist[0],"v")==0) {

            if (vert_count%10==0) {
                lastvert->next = malloc(sizeof(struct linqvert));

                lastvert->next->last = lastvert;
                lastvert->next->next = NULL;

                lastvert->next->uto = 0;

                //printf("%i\n",amount_of_phrases);
            
                lastvert = lastvert->next;
            }

            lastvert->points[lastvert->uto] = atof(splittedlist[1]);
            lastvert->points[lastvert->uto+1] = atof(splittedlist[2]);
            lastvert->points[lastvert->uto+2] = atof(splittedlist[3]);

            lastvert->uto +=3;
            vert_count++;
        }
        if (strcmp(splittedlist[0],"vt")==0) {
            if (vert_uv_count%15==0) {
                lastuvvert->next = malloc(sizeof(struct linqvert));

                lastuvvert->next->last = lastuvvert;
                lastuvvert->next->next = NULL;

                lastuvvert->next->uto = 0;

                //printf("%i\n",amount_of_phrases);
            
                lastuvvert = lastuvvert->next;
            }

            lastuvvert->points[lastuvvert->uto] = atof(splittedlist[1]);
            lastuvvert->points[lastuvvert->uto+1] = atof(splittedlist[2]);

            lastuvvert->uto +=2;
            vert_uv_count++;
        }
        if (strcmp(splittedlist[0],"vn")==0) {
            if (vert_norm_count%10==0) {
                lastnormvert->next = malloc(sizeof(struct linqvert));

                lastnormvert->next->last = lastnormvert;
                lastnormvert->next->next = NULL;

                lastnormvert->next->uto = 0;

                //printf("%i\n",amount_of_phrases);
            
                lastnormvert = lastnormvert->next;
            }

            lastnormvert->points[lastnormvert->uto] = atof(splittedlist[1]);
            lastnormvert->points[lastnormvert->uto+1] = atof(splittedlist[2]);
            //lastnormvert->points[lastnormvert->uto+2] = atof(splittedlist[3]);

            lastnormvert->uto +=3;
            vert_norm_count++;
        }

        
        if (strcmp(splittedlist[0],"f")==0) {
            lastface->next = calloc(1,sizeof(struct linqface));

            lastface->next->next = NULL;
            lastface->next->last = lastface;

            lastface->data = malloc(sizeof(struct facedata)*(amount_of_phrases - 1));

            lastface->vert_count = amount_of_phrases - 1;

            lastface->material = current_mat;

            lastface->is_smooth = fsmooth;

            if (lastface->vert_count!=3) {
                has_ngons=1;
            }
            for (int i=1;i<amount_of_phrases;i++) {
                int tcount = count_chars(splittedlist[i],faceseptoken)+1;

                //printf("%i\n",tcount);

                if (tcount==1) { // more common than youd think
                    lastface->data[i-1].vert_id = atoi_cool(splittedlist[i]);
                } else {
                    char** splitted;
                    //printf("%s\n",splittedlist[i]);
                    splitted = malloc(sizeof(char*)*tcount);
                    splitted[0] = strtok(splittedlist[i], faceseptoken);
                    for (int x=1;x<tcount;x++) {
                        splitted[x] = strtok(NULL, faceseptoken);
                    }
                    /*for (int x=0;x<tcount;x++) {
                        printf("thing: '%c'\n",*splitted[x]);
                    }*/

                    //printf("tcount: %i, thing: %s",tcount,splittedlist[i]);

                    lastface->data[i-1].vert_id = atoi_cool(splitted[0]);
                    if (strlen(splitted[1])>0) {
                        lastface->data[i-1].uv_id = atoi_cool(splitted[1]);
                        has_uvs=1;
                    }
                    if (tcount==2) {
                        lastface->data[i-1].norm_id = atoi_cool(splitted[2]);
                        has_norms=1;
                    }
                    free(splitted);
                }
            }
            
            lastface = lastface->next;

            face_count++;
        }

        if (strcmp(splittedlist[0],"mtllib")==0) {
            int namelen = strlen(splittedlist[1]);
            matnames[mat_count] = malloc(namelen+1);
            snprintf(matnames[mat_count],namelen,splittedlist[1]);
            mat_count++;
        }

        /*if (strcmp(splittedlist[0],"mtllib")==0) {
            int namelen = strlen(splittedlist[1]);
            matnames[mat_count] = malloc(namelen+1);
            snprintf(matnames[mat_count],namelen,splittedlist[1]);
            mat_count++;
        }*/
        
        free(splittedlist);
    }

    if (has_ngons!=0) {
        printf("\x1b[1;33mWarning: OBJ models with n-gons are not supported yet, the faces will be ommited in the conversion process\x1b[m\n");
    }

    if (g_debug)
        printf("Optimising data structures\n");
    
    // store them in a non-linq list for fast acess
    struct vec3* allverts = malloc(sizeof(struct vec3)*vert_count);
    struct vec3* allnorms = malloc(sizeof(struct vec3)*vert_norm_count);
    struct vec2* alluvs = malloc(sizeof(struct vec2)*vert_uv_count);

    struct linqvert* cur = &verts;
    int currentvert=0;
    //printf("asd: %f\n",ceil(vert_count/10.0));
    /*for (int i=0;i<(int)ceil((double)(vert_count/10.0));i++) {
        for (int x=0;x<30;x+=3) {
            allverts[currentvert].x = cur->points[x];
            allverts[currentvert].y = cur->points[x+1];
            allverts[currentvert].z = cur->points[x+2];
            printf("asdf: %f\n",cur->points[x]);
            currentvert++;
        }
        cur=cur->next;
    }*/
    for (int i=0;i<vert_count;i++) {
        allverts[i].x = cur->points[(i%10)*3];
        allverts[i].y = cur->points[(i%10)*3+1];
        allverts[i].z = cur->points[(i%10)*3+2];
        if (i%10==0) {
            cur=cur->next;
        }
    }
    cur = &uvverts;
    /*for (int i=0;i<(int)ceil((double)(vert_uv_count/15.0));i++) {
        for (int x=0;x<15;x++) {
            alluvs[currentvert].x = cur->points[x];
            alluvs[currentvert].y = cur->points[x+1];
            currentvert++;
        }
        cur=cur->next;
    }*/
    for (int i=0;i<vert_uv_count;i++) {
        alluvs[i].x = cur->points[(i%15)*2];
        alluvs[i].y = cur->points[(i%15)*2+1];
        if (i%15==0) {
            cur=cur->next;
        }
    }
    cur = &normverts;
    /*currentvert=0;
    for (int i=0;i<(int)ceil((double)(vert_norm_count/10.0));i++) {
        for (int x=0;x<10;x++) {
            allnorms[currentvert].x = cur->points[x];
            allnorms[currentvert].y = cur->points[x+1];
            allnorms[currentvert].z = cur->points[x+2];
            currentvert++;
        }
        cur=cur->next;
    }*/
    for (int i=0;i<vert_norm_count;i++) {
        allnorms[i].x = cur->points[(i%10)*3];
        allnorms[i].y = cur->points[(i%10)*3+1];
        allnorms[i].z = cur->points[(i%10)*3+2];
        if (i%10==0) {
            cur=cur->next;
        }
    }

    if (g_debug)
        printf("Translating to an SGMable structure (materials not supported yet)\n");

    // allocate and write some values

    sgm->number_of_materials = 1;
    sgm->materials = malloc(sizeof(struct sgmmaterial));
    if (has_uvs!=0) {
        sgm->materials->uvs = malloc(sizeof(struct sgmuvset));
        sgm->materials->number_of_uv_sets = 1;
        sgm->materials->uvs->number_of_textures=0; // change later somehow
        sgm->materials->uvs->textures=NULL;
    } else {
        sgm->materials->uvs=NULL;
        sgm->materials->number_of_uv_sets = 0;
    }

    sgm->materials->material_id=0;
    sgm->materials->number_of_colors = 1;
    sgm->materials->colors=malloc(sizeof(struct sgmcolor));
    sgm->materials->colors->color_type_hint = 4;
    sgm->materials->colors->r = 1.0;
    sgm->materials->colors->g = 1.0;
    sgm->materials->colors->b = 1.0;
    sgm->materials->colors->a = 1.0;
    

    sgm->number_of_meshes = 1;

    sgm->meshes = malloc(sizeof(struct sgmmesh));

    struct vertdict {
        int original;
        int real;
    };
    
    int used_verts_count=0;
    int used_norm_verts_count=0;
    int used_uv_verts_count=0;
    struct vertdict* used_verts = malloc(sizeof(struct vertdict)*vert_count);
    struct vertdict* used_norm_verts = malloc(sizeof(struct vertdict)*vert_count);
    struct vertdict* used_uv_verts = malloc(sizeof(struct vertdict)*vert_count);

    int real_vert_count=0;
    struct linqface* current = &faces;
    for (int i=0;i<face_count;i++) {
        if (current->vert_count>3) {
            continue;
        }
        for (int x=0;x<current->vert_count;x++) {
            struct facedata* fda = &current->data[x];

            int pos_match=0;
            int norm_match=0;
            int uv_match=0;

            int pos_matched=0;
            int norm_matched=0;
            int uv_matched=0;

            for (int v=0;v<used_verts_count;v++) {
                if (used_verts[v].original==fda->vert_id) {
                    pos_match=v;
                    pos_matched=1;
                    break;
                }
            }
            if (has_norms!=0){
                for (int v=0;v<used_norm_verts_count;v++) {
                    if (used_norm_verts[v].original==fda->norm_id) {
                        norm_match=v;
                        norm_matched=1;
                        break;
                    }
                }
            }
            if (has_uvs!=0){
                for (int v=0;v<used_uv_verts_count;v++) {
                    if (used_uv_verts[v].original==fda->uv_id) {
                        uv_match=v;
                        uv_matched=1;
                        break;
                    }
                }
            }

            if (pos_matched==0||((vert_norm_count<1)?norm_matched==0:0)||((vert_uv_count<1)?uv_matched==0:0)||current->is_smooth==0) {
                real_vert_count++;
            }
        }
        current=current->next;
    }
    real_vert_count++;

    sgm->meshes->vertex_data = calloc(1,sizeof(struct sgmvertex)*real_vert_count);


    sgm->meshes->mesh_id=0;
    sgm->meshes->used_materials_id=0;
    sgm->meshes->color_channel_count = 0;
    sgm->meshes->has_tangents=0;
    sgm->meshes->has_bones=0;
    sgm->meshes->number_of_vertices=vert_count;

    if (has_uvs) {
        sgm->meshes->texcoord_count=1;
    } else {
        sgm->meshes->texcoord_count=0;
    }


    int index_count=0;

    // count indices
    current=&faces;
    for (int i=0;i<face_count;i++) {
        index_count+=current->vert_count;
        current=current->next;
    }

    sgm->meshes->number_of_indices=index_count;

    if (index_count>65535) {
        sgm->meshes->index_size=4;
        sgm->meshes->indices_2=NULL;
        sgm->meshes->indices_4=malloc(sizeof(uint32)*index_count);
    } else {
        sgm->meshes->index_size=2;
        sgm->meshes->indices_4=NULL;
        sgm->meshes->indices_2=malloc(sizeof(uint16)*index_count);
    }


    
    


    sgm->has_animation = 0;
    sgm->animfilename_length = 0;


    

    // idea: each face in an obj has indexes for uv norm and pos but in an sgm each vert has all of that data in it already, so create some lists that link that obj indices to the sgm vert number for later re-using ;)

    if (g_debug)
        printf("Writing faces and verts\n");

    used_verts_count=0;
    used_norm_verts_count=0;
    used_uv_verts_count=0;
    used_verts = malloc(sizeof(struct vertdict)*vert_count);
    used_norm_verts = malloc(sizeof(struct vertdict)*vert_count);
    used_uv_verts = malloc(sizeof(struct vertdict)*vert_count);

    int written_index_count=0;
    int written_vertex_count=0;

    current = &faces;
    for (int i=0;i<face_count;i++) {
        if (current->vert_count>3) {
            continue;
        }
        for (int x=0;x<current->vert_count;x++) {
            struct facedata* fda = &current->data[x];

            int pos_match=0;
            int norm_match=0;
            int uv_match=0;

            int pos_matched=0;
            int norm_matched=1;
            int uv_matched=1;

            for (int v=0;v<used_verts_count;v++) {
                if (used_verts[v].original==fda->vert_id) {
                    pos_match=v;
                    pos_matched=1;
                    break;
                }
            }
            if (has_norms!=0){
                norm_matched=0;
                for (int v=0;v<used_norm_verts_count;v++) {
                    if (used_norm_verts[v].original==fda->norm_id) {
                        norm_match=v;
                        norm_matched=1;
                        break;
                    }
                }
            }
            if (has_uvs!=0){
                uv_matched=0;
                for (int v=0;v<used_uv_verts_count;v++) {
                    if (used_uv_verts[v].original==fda->uv_id) {
                        uv_match=v;
                        uv_matched=1;
                        break;
                    }
                }
            }

            if (uv_matched!=0&&norm_matched!=0&&pos_matched!=0&&current->is_smooth) {
                if (sgm->meshes->index_size==2) {
                    sgm->meshes->indices_2[written_index_count]=pos_match;
                } else {
                    sgm->meshes->indices_4[written_index_count]=pos_match;
                }
                written_index_count++;
            } else {
                struct sgmvertex* vert = &sgm->meshes->vertex_data[written_vertex_count];
                
                vert->position_x = allverts[fda->vert_id-1].x;
                vert->position_y = allverts[fda->vert_id-1].y;
                vert->position_z = allverts[fda->vert_id-1].z;
                
                used_verts[used_verts_count].original=fda->vert_id;
                used_verts_count++;
                
                //printf("%f\n",allverts[fda->vert_id-1].x);

                if (has_norms){
                    vert->normal_x = allnorms[fda->norm_id-1].x;
                    vert->normal_y = allnorms[fda->norm_id-1].y;
                    vert->normal_z = allnorms[fda->norm_id-1].z;

                    used_norm_verts[used_norm_verts_count].original=fda->norm_id;
                    used_norm_verts_count++;
                }

                if (has_uvs){
                    vert->uvs = malloc(sizeof(struct sgmuvpos));

                    vert->uvs->u = alluvs[fda->uv_id-1].x;
                    vert->uvs->v = alluvs[fda->uv_id-1].y;

                    used_uv_verts[used_uv_verts_count].original=fda->uv_id;
                    used_uv_verts_count++;
                }

                if (sgm->meshes->index_size==2) {
                    sgm->meshes->indices_2[written_index_count]=written_vertex_count;
                } else {
                    sgm->meshes->indices_4[written_index_count]=written_vertex_count;
                }
                written_index_count++;
                
                vert->has_color=0;
                vert->has_tangents=0;
                vert->has_bones=0;
                written_vertex_count++;
            }

            

        }
        current=current->next;
    }

    if (g_debug)
        printf("Written index count: %i\n",written_index_count);

    //free(used_verts);
    free(used_norm_verts);
    //free(used_uv_verts);

    // free memory
    if (g_debug)
        printf("Freeing storage\n");

    free(allverts);
    free(allnorms);
    free(alluvs);

    struct linqvert* selected = lastvert->last;
    for (int i=0;i<ceil(vert_count/10);i++) {
        free(selected->next);
        selected=selected->last;
    }
    
    selected = lastuvvert->last;
    for (int i=0;i<ceil(vert_uv_count/10);i++) {
        free(selected->next);
        selected=selected->last;
    }
    
    selected = lastnormvert->last;
    for (int i=0;i<ceil(vert_norm_count/10);i++) {
        free(selected->next);
        selected=selected->last;
    }

    struct linqface* selectedface = lastface->last;
    for (int i=0;i<face_count-1;i++) {
        free(selectedface->next->data);
        
        free(selectedface->next);
        selectedface=selectedface->last;
    }
    if (face_count>0) {
        free(faces.data);
    }

    for (int i=0;i<mat_count;i++) {
        free(matnames[i]);
    }

    if (g_debug)
        printf("Obj file summary:\nVert count:     %i\nFace count:     %i\nMaterial count: %i\n",vert_count,face_count,mat_count);

    fclose(fileptr);
}



//void obj_to_sgmable(char* filename, 

int main(int argc, char* argv[]) {

    char input_file[200];
    char output_file[200];

    int input_file_valid = 0;
    int output_file_valid = 0;

    /*
        0 = Wavefront obj

        none other supported
    */
    int filetype = 0;

    for (int i=0;i<argc;i++) {
        if (strcmp(argv[i],"-v")==0||strcmp(argv[i],"--version")==0) {
            return print_version();
        }
        if (strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0) {
            return print_help();
        }
        if (strcmp(argv[i],"-d")==0 || strcmp(argv[i],"--debug")==0) {
            g_debug=1;
        }
        if (strcmp(argv[i],"-w")==0) {
            if (i+2>argc) {
                err("-w [Input filename]");
                return 0;
            }
            input_file_valid=1;
            sprintf(input_file,argv[i+1]);
        }
        
        if (strcmp(argv[i],"-o")==0) {
            if (i+2>argc) {
                err("-o [Output filename]");
                return 0;
            }
            output_file_valid=1;
            sprintf(output_file,argv[i+1]);
        }
    }

    /*if (input_file!=0) {
        printf("%s\n",input_file);
    }*/
    if (input_file_valid==0&&output_file_valid==0) {
        printf("Sgmtool %s\nUse --help for help menu.\n",VERSION);
    }
    if (input_file_valid==1&&output_file_valid==1) {

        if (g_debug)
            printf("Creating sgm object\n");
        
        // cool thang
        struct sgmfile sgm;

        memset(&sgm, 0, sizeof(sgm));
        
        sgm_fromwavefront(input_file, &sgm);
        
        return serialize_sgmfile(output_file, sgm, 0, 0, 0, 0);

    }
    

    return 0;
}
