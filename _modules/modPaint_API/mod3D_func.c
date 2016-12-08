#include "mod3D_func.h"
#include "board.h"
#include "halPaint.h"
#include "modPaint.h"

#include "debug.h"
 
 


// float cos_(unsigned char angle)
// {
//     unsigned char a1, a2;
//     a1 = angle & 0x0f;
//     a2 = angle >> 4 ; // определяем сектор
//     if(a2 == 0) return (float)(cospi[a1]);
//     if(a2 == 1) return (float)(cospi[16-a1] * (-1));
//     if(a2 == 2) return (float)(cospi[a1] * (-1));
//     if(a2 == 3) return (float)(cospi[16-a1] );
// }

// float sin_(unsigned char angle)
// {
//     unsigned char a1, a2;
//     a1 = angle & 0x0f;
//     a2 = angle >> 4 ;
//     if(a2 == 0) return (float)(cospi[16-a1]);
//     if(a2 == 1) return (float)(cospi[a1]);
//     if(a2 == 2) return (float)(cospi[16-a1] * (-1));
//     if(a2 == 3) return (float)(cospi[a1] * (-1));
// }

// ////////////// soft 3D ///////////////
#define RANGE 1024.0F


// Всего возможно вернуть 16*4 значений
const uint32_t cospi[]= {
    RANGE * 1.0F,  //0
    RANGE * 0.99518473F,
    RANGE * 0.98078528F,
    RANGE * 0.95694034F,
    RANGE * 0.92387953F,
    RANGE * 0.88192126F,
    RANGE * 0.83146961F,
    RANGE * 0.77301045F,//7
    RANGE * 0.70710678F,
    RANGE * 0.63439328F,
    RANGE * 0.55557023F,
    RANGE * 0.47139674F,
    RANGE * 0.38268343F,
    RANGE * 0.29028468F,
    RANGE * 0.19509032F,
    RANGE * 0.09801714F,//15
    RANGE * 0.0F  //16
};

int32_t _cos (uint16_t angle)
{
    uint32_t a1, a2;
    a1 = angle & 0x000F;
    a2 = angle >> 4 ; // определяем сектор
    switch ( a2 )
    {
        case 0: a2 = (cospi[a1]); break;
        case 1: a2 = (cospi[16 - a1] * (-1)); break;
        case 2: a2 = (cospi[a1] * (-1)); break;
        case 3: a2 = (cospi[16 - a1]); break;
        default: a2 = 0; break;
    }
    
    return a2;
}


int32_t _sin (uint16_t angle)
{
    uint32_t a1, a2;
    a1 = angle & 0x000F;
    a2 = angle >> 4 ; // определяем сектор
    switch ( a2 )
    {
        case 0: a2 = (cospi[16 - a1]); break;
        case 1: a2 = (cospi[a1]); break;
        case 2: a2 = (cospi[16 - a1] * (-1)); break;
        case 3: a2 = (cospi[a1] * (-1)); break;
        default: a2 = 0; break;
    }
    
    return a2;
}



//////////////////////////////////////////////////////////////////
// http://we.easyelectronics.ru/KT3012/soft-render.html#comment118245
// http://pmg.org.ru/basic3d/
// http://pmg.org.ru/basic3d/graph.htm
// http://pmg.org.ru/basic3d/struct.htm
// http://habrahabr.ru/post/107268/


//Константы
#define CUBE_SIZE   (100 / 6) //(длина ребра)/2
#define VERTEX_NUM    8 // всего вершин
#define FACES_NUM   12 // количество граней

/** массивы содержат номера начальных (s1) и конечных (f1) точек,
 *  по ним соединяем все и рисуем грани(сетку) 
 */
const uint8_t cube_s1[FACES_NUM] = {
    0, 1, 2, 3, 0, 1, 2, 3, 4 ,5, 6, 7
};

const uint8_t cube_f1[FACES_NUM] = {
    1, 2, 3, 0, 4, 5, 6, 7, 5, 6, 7, 4 //7
}; 

//точки, лучше поменять на long
const int32_t cube_x[VERTEX_NUM] = {
    -CUBE_SIZE ,
     CUBE_SIZE,
     CUBE_SIZE,
    -CUBE_SIZE,
    -CUBE_SIZE,
     CUBE_SIZE,
     CUBE_SIZE,
    -CUBE_SIZE,
};
const int32_t cube_y[VERTEX_NUM] = {
     CUBE_SIZE,
     CUBE_SIZE,
    -CUBE_SIZE,
    -CUBE_SIZE,
     CUBE_SIZE,
     CUBE_SIZE,
    -CUBE_SIZE,
    -CUBE_SIZE,
};
const int32_t cube_z[VERTEX_NUM] = {
     CUBE_SIZE,
     CUBE_SIZE,
     CUBE_SIZE,
     CUBE_SIZE,
    -CUBE_SIZE,
    -CUBE_SIZE,
    -CUBE_SIZE,
    -CUBE_SIZE,
};

#define TRIANGLE_NUM 12
const uint8_t cube_t1[] = {
    0, 2, 1, 6, 5, 7, 4, 3, 0, 5, 3, 6,
};
const uint8_t cube_t2[] = {
    1, 3, 5, 2, 4, 6, 0, 7, 4, 1, 2, 7,
}; 
const uint8_t cube_t3[] = {
    3, 1, 2, 5, 6, 4, 7, 0, 1, 4, 7, 2,
}; 

typedef struct _3D_VERTEX_TYPE_ {
/** углы поворота по осям, от 0 до 63 включительно,
 *  это полный оборот на 360
 */
    uint32_t    *x;
    uint32_t    *y;
    uint32_t    *z;
    
}_3D_VERTEX_TYPE;

typedef struct _3D_FACE_TYPE_ {
/** углы поворота по осям, от 0 до 63 включительно,
 *  это полный оборот на 360
 */
    uint8_t    *s;
    uint8_t    *f;
    
    uint8_t    *t1;
    uint8_t    *t2;
    uint8_t    *t3;
    
}_3D_FACE_TYPE;

typedef struct _3D_OBJECT_TYPE_ {
/** углы поворота по осям, от 0 до 63 включительно,
 *  это полный оборот на 360
 */
    int32_t    dir_x;
    int32_t    dir_y;
    int32_t    dir_z;
    _3D_VERTEX_TYPE vertices; // вершины
    _3D_FACE_TYPE   face; // грани
    uint8_t       numvertices;
    uint8_t       numfaces;
    
    //uint8_t       *textptr; // текстура
}_3D_OBJECT_TYPE;
  
_3D_OBJECT_TYPE _3DCube; // Создали


void mod3D_init (void)//_3D_OBJECT_TYPE *obj)
{
    // собираем объект
    _3DCube.dir_x = 0; 
    _3DCube.dir_y = 0;
    _3DCube.dir_z = 0; 

    _3DCube.vertices.x = (uint32_t *) &cube_x[0];
    _3DCube.vertices.y = (uint32_t *) &cube_y[0];
    _3DCube.vertices.z = (uint32_t *) &cube_z[0];
    
    _3DCube.face.s = (uint8_t *) &cube_s1[0];
    _3DCube.face.f = (uint8_t *) &cube_f1[0];
    
    _3DCube.face.t1 = (uint8_t *) &cube_t1[0];
    _3DCube.face.t2 = (uint8_t *) &cube_t2[0];
    _3DCube.face.t3 = (uint8_t *) &cube_t3[0];
    
    _3DCube.numvertices =  VERTEX_NUM;
    _3DCube.numfaces    =  TRIANGLE_NUM; // ; FACES_NUM
}



static uint32_t f;

void _3dObj_draw_faces (_3D_OBJECT_TYPE *obj, 
                uint32_t x_pos, 
                uint32_t y_pos,
                uint32_t z_pos,
                color_t color)
{
    //uint8_t angle; //буфер для угла
    int32_t x1, y1, z1;  // координаты точек от центра объекта
    
    uint16_t x2d[VERTEX_NUM], y2d[VERTEX_NUM]; // экранные точки
    int32_t  x[VERTEX_NUM], y[VERTEX_NUM], z[VERTEX_NUM];
    uint32_t i; // временные переменные
    int32_t sin, cos;
    
    for (i = 0; i < obj->numvertices; i++) //по X
    {
        x[i] = obj->vertices.x[i];
        y[i] = obj->vertices.y[i];
        z[i] = obj->vertices.z[i];
    }
    
    
    //Вращаем наши точки, фактически матрицы вращения упрощенные
    cos = _cos (obj->dir_x);
    sin = _sin (obj->dir_x);
    for (i = 0; i < obj->numvertices; i++) // по X
    {
        y1 = y[i];
        z1 = z[i];
        y[i] =  (cos * y1 - sin * z1) / RANGE;
        z[i] =  (cos * z1 + sin * y1) / RANGE;
    }
    
    cos = _cos (obj->dir_y);
    sin = _sin (obj->dir_y);
    for (i = 0; i < obj->numvertices; i++) // по Y
    {
        x1 = x[i];
        z1 = z[i];
        x[i] =  (cos * x1 + sin * z1) / RANGE;
        z[i] = (-sin * x1 + cos * z1) / RANGE;
    }
    
    cos = _cos (obj->dir_z);
    sin = _sin (obj->dir_z);
    for (i = 0; i < obj->numvertices; i++) // и Z незабыть!
    {
        x1 = x[i];
        y1 = y[i];
        x[i] =  (cos * x1 - sin * y1) / RANGE;
        y[i] =  (cos * y1 + sin * x1) / RANGE;
    }

    //Трансформация координат вершин в экранные
    for (i = 0; i < obj->numvertices; i++)
    {
        //1000 и 1200 определяют расстояние от  объекта до камеры и 
        f = (RANGE * 100) / (110 -  (z[i] ) - z_pos);
        // рисуем объект с центром в по-центре экрана
        x2d[i] = (uint16_t)((int32_t)(f * x[i] / RANGE) + x_pos);
        y2d[i] = (uint16_t)((int32_t)(f * y[i] / RANGE) + y_pos);
    }

    //Рисуем ребра/сетку
    for(i = 0; i < obj->numfaces; i++) // рисуем отрезок
    {
        paint_setColor (color);
        paint_line ( x2d[_3DCube.face.s[i]], y2d[obj->face.s[i]], x2d[obj->face.f[i]], y2d[obj->face.f[i]]); 
    }

}

extern int16_t X, Y, Z;
void mod3D_run (void)
{
    //clear
    _3dObj_draw_faces (&_3DCube, SCREEN_W / 2, SCREEN_H / 2 + 37, 33, COLOR_WHITE);

    //Инкрементируем углы
    /*
    if(_3DCube.dir_x++ >= 63) _3DCube.dir_x = 0;
    if(_3DCube.dir_y++ >= 63) _3DCube.dir_y = 0;
    if(_3DCube.dir_z++ >= 63) _3DCube.dir_z = 0;
    */

    _3DCube.dir_x = X;
    _3DCube.dir_y = Y;
    _3DCube.dir_z = Z;

    if(_3DCube.dir_x > 63) _3DCube.dir_x = 63;
    if(_3DCube.dir_y > 63) _3DCube.dir_y = 63;
    if(_3DCube.dir_z > 63) _3DCube.dir_z = 63;

    if(_3DCube.dir_x < 0) _3DCube.dir_x = 0;
    if(_3DCube.dir_y < 0) _3DCube.dir_y = 0;
    if(_3DCube.dir_z < 0) _3DCube.dir_z = 0;

    _3dObj_draw_faces (&_3DCube, SCREEN_W / 2, SCREEN_H / 2 + 37, 33, COLOR_BLUE);
}
