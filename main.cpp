#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  include "glut.h"
#  include "glext.h"
PFNGLACTIVETEXTUREPROC glActiveTexture;
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

/*
** 光源
*/
static const GLfloat lightpos[] = { 5.0, 4.0, 3.0, 0.0 }; /* 位置　　　 */
static const GLfloat lightcol[] = { 1.0, 1.0, 1.0, 1.0 }; /* 直接光強度 */
static const GLfloat lightamb[] = { 0.1, 0.1, 0.1, 1.0 }; /* 環境光強度 */

/*
** テクスチャ
*/
#define TEXWIDTH  256                      /* テクスチャの幅　　　 */
#define TEXHEIGHT 256                      /* テクスチャの高さ　　 */
static const char texture1[] = "dot.raw";  /* テクスチャファイル名 */

/*
** フレネル関数
*/
static float fresnel(float c)
{
  const float n = 1.5; /* 屈折率の比 */
  const float g = sqrt(n * n + c * c - 1.0);
  const float gpc = g + c;
  const float gmc = g - c;
  const float gpc1 = c * gpc - 1.0;
  const float gmc1 = c * gmc + 1.0;
  const float gc = gmc / gpc;
  const float gc1 = gpc1 / gmc1;
  return 0.5 * gc * gc * (1.0 + gc1 * gc1);
}

/*
** 初期化
*/
static void init(void)
{
  /* テクスチャの読み込みに使う配列 */
  GLubyte texture[TEXHEIGHT * TEXWIDTH * 4];
  FILE *fp;
  
  /* テクスチャ画像はワード単位に詰め込まれている */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  
  /* テクスチャ画像の読み込み */
  if ((fp = fopen(texture1, "rb")) != NULL) {
    fread(texture, sizeof texture, 1, fp);
    fclose(fp);
  }
  
  /* テクスチャの割り当て */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, texture);
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  /* テクスチャ環境 */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  
#if defined(WIN32)
  glActiveTexture =
    (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
#endif
  
  /* テクスチャユニット１に切り替える */
  glActiveTexture(GL_TEXTURE1);
  
  /* フレネル関数のテーブル */
  GLfloat table[128][2];
  
  /* フレネル関数のテーブル作成 */
  for (int i = 0; i < 128; ++i) {
    table[i][0] = 1.0;
    table[i][1] = fresnel((float)i / 127.0f);
  }
  
  /* フレネル関数のテーブルを一次元テクスチャとして割り当て */
  glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE_ALPHA, 128, 0,
    GL_LUMINANCE_ALPHA, GL_FLOAT, table);
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  
  /* テクスチャユニット１のテクスチャ環境 */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

  /* 法線ベクトルのｚ成分をテクスチャ座標として補間する */
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP);
  
  /* テクスチャ座標の自動生成を有効にする */
  glEnable(GL_TEXTURE_GEN_R);
  
  /* テクスチャのパラメータ r を s と交換する */
  static const GLdouble mat[] = {
    0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0,
  };
  glMatrixMode(GL_TEXTURE);
  glLoadMatrixd(mat);
  glMatrixMode(GL_MODELVIEW);
  
  /* テクスチャユニット２に切り替える */
  glActiveTexture(GL_TEXTURE2);
  
  for (int i = 0; i < 6; ++i) {
    /* テクスチャファイル名 */
    static const char *textures[] = {
      "room2ny.raw", /* 下 */
      "room2nz.raw", /* 裏 */
      "room2px.raw", /* 右 */
      "room2pz.raw", /* 前 */
      "room2nx.raw", /* 左 */
      "room2py.raw", /* 上 */
    };
    /* テクスチャのターゲット名 */
    static const int target[] = {
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
      GL_TEXTURE_CUBE_MAP_POSITIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    };
    
    if ((fp = fopen(textures[i], "rb")) != NULL) {
      /* テクスチャ画像の読み込み */
      fread(texture, 128 * 128 * 4, 1, fp);
      fclose(fp);
      
      /* キューブマッピングのテクスチャの割り当て */
      glTexImage2D(target[i], 0, GL_RGBA, 128, 128, 0, 
        GL_RGBA, GL_UNSIGNED_BYTE, texture);
    }
  }
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  /* テクスチャユニット２のテクスチャ環境 */
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
#if 0
  static const GLfloat blend[] = { 1.0, 1.0, 1.0, 0.5 };
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blend);
#else
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);
#endif
  
  /* キューブマッピング用のテクスチャ座標を生成する */
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
  
  /* テクスチャユニット０に戻す */
  glActiveTexture(GL_TEXTURE0);
  
  /* 初期設定 */
  glClearColor(0.3, 0.3, 1.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  /* 光源の初期設定 */
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightcol);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
}

/* トラックボール処理用関数の宣言 */
#include "trackball.h"

/* 箱を描く関数の宣言 */
#include "box.h"

/*
** シーンの描画
*/
static void scene(void)
{
  static const GLfloat color[] = { 1.0, 1.0, 1.0, 1.0 };  /* 材質 (色) */
  
  /* 材質の設定 */
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
  
  /* テクスチャマッピング開始 */
  glEnable(GL_TEXTURE_2D);
  
  /* テクスチャユニット１に切り替える */
  glActiveTexture(GL_TEXTURE1);
  
  /* １次元テクスチャマッピング開始 */
  glEnable(GL_TEXTURE_1D);
  
  /* テクスチャユニット２に切り替える */
  glActiveTexture(GL_TEXTURE2);
  
  /* キューブマッピング開始 */
  glEnable(GL_TEXTURE_CUBE_MAP);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);
  
  /* トラックボール処理による回転 */
  glMultMatrixd(trackballRotation());

  /* 箱を描く */
#if 0
  box(1.0, 1.0, 1.0);
#else
  glutSolidTeapot(1.2);
#endif

  /* キューブマッピング終了 */
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);
  glDisable(GL_TEXTURE_CUBE_MAP);
  
  /* テクスチャユニット１に切り替える */
  glActiveTexture(GL_TEXTURE1);
  
  /* １次元テクスチャマッピング終了 */
  glDisable(GL_TEXTURE_1D);
  
  /* テクスチャユニット０に戻す */
  glActiveTexture(GL_TEXTURE0);
  
  /* テクスチャマッピング終了 */
  glDisable(GL_TEXTURE_2D);
}


/****************************
** GLUT のコールバック関数 **
****************************/

static void display(void)
{
  /* 画面クリア */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* モデルビュー変換行列の設定 */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  /* 光源の位置を設定 */
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  
  /* 視点の移動（物体の方を奥に移動）*/
  glTranslated(0.0, 0.0, -5.0);
  
  /* シーンの描画 */
  scene();
  
  /* ダブルバッファリング */
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  /* トラックボールする範囲 */
  trackballRegion(w, h);
  
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);
  
  /* 透視変換行列の指定 */
  glMatrixMode(GL_PROJECTION);
  
  /* 透視変換行列の初期化 */
  glLoadIdentity();
  gluPerspective(40.0, (double)w / (double)h, 1.0, 100.0);
}

static void idle(void)
{
  /* 画面の描き替え */
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  switch (button) {
  case GLUT_LEFT_BUTTON:
    switch (state) {
    case GLUT_DOWN:
      /* トラックボール開始 */
      trackballStart(x, y);
      glutIdleFunc(idle);
      break;
    case GLUT_UP:
      /* トラックボール停止 */
      trackballStop(x, y);
      glutIdleFunc(0);
      break;
    default:
      break;
    }
    break;
    default:
      break;
  }
}

static void motion(int x, int y)
{
  /* トラックボール移動 */
  trackballMotion(x, y);
}

static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
  case 'Q':
  case '\033':
    /* ESC か q か Q をタイプしたら終了 */
    exit(0);
  default:
    break;
  }
}

/*
** メインプログラム
*/
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  return 0;
}
