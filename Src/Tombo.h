#ifndef TOMBO_H
#define TOMBO_H

///////////////////////////////////////
// �E�B���h�E���b�Z�[�W
///////////////////////////////////////

// MWM_OPEN_REQUEST : �����̃I�[�v���v��
// WPARAM : �I�v�V����
// LPARAM : �I�[�v�������ׂ��m�[�h�ւ̃|�C���^
#define MWM_OPEN_REQUEST (0x8000 + 1)

// MWM_OPEN_REQUEST �I�v�V����
#define OPEN_REQUEST_MDVIEW_ACTIVE 0
#define OPEN_REQUEST_MSVIEW_ACTIVE 1
#define OPEN_REQUEST_NO_ACTIVATE_VIEW 2

// MWM_RAISE_MAINFRAME : �g�b�v���x���E�B���h�E��Raise���čđO�ʂɂ����Ă���
// WPARAM, LPARAM: 0
#define MWM_RAISE_MAINFRAME (0x8000 + 2)

///////////////////////////////////////
// ���ʒ�`
///////////////////////////////////////

#define TOMBO_MAIN_FRAME_WINDOW_CLSS TEXT("TomboMainFrmWnd")

#define TOMBO_APP_NAME TEXT("Tombo")

// �����񃊃\�[�X�̍ő咷
#define MESSAGE_MAX_SIZE 1024

// ���X�g�r���[�����C���[�W���X�g
#define IMG_FOLDER 0
#define IMG_FOLDER_SEL 1
#define IMG_ARTICLE 2
#define IMG_ARTICLE_ENCRYPTED 3
#define IMG_ARTICLE_MASKED 4
#define IMG_ARTICLE_ENC_MASKED 5
#define IMG_FOLDER_MASKED 6
#define IMG_FOLDER_SEL_MASKED 7

extern HINSTANCE g_hInstance;

#include "Logger.h"
extern Logger *g_pLogger;

int TomboMessageBox(HWND hWnd, LPCTSTR pText, LPCTSTR pCaption, UINT uType); 
extern BOOL bDisableHotKey;

// �A�v���P�[�V�����{�^���̒�`
#define APP_BUTTON1 (0xC1)
#define APP_BUTTON2 (0xC2)
#define APP_BUTTON3 (0xC3)
#define APP_BUTTON4 (0xC4)
#define APP_BUTTON5 (0xC5)

#define APPBUTTON_ACTION_DISABLE 0
#define APPBUTTON_ACTION_ENABLE  1

#endif
