#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef TOMBO_LANG_ENGLISH
#define MSG(J,E) TEXT(E)
#else
#define MSG(J,E) TEXT(J)
#endif

#define MSG_TOOLTIPS_NEWMEMO	MSG("�V�K����", "New Memo")
#define MSG_TOOLTIPS_RETURNLIST	MSG("�ꗗ�ɖ߂�", "Return to list")
#define MSG_TOOLTIPS_SAVE		MSG("�ۑ�", "Save")
#define MSG_TOOLTIPS_INSDATE1	MSG("���t1�̑}��", "Insert date1")
#define MSG_TOOLTIPS_INSDATE2	MSG("���t2�̑}��", "Insert date2")
#define MSG_MEMO                MSG("����", "Memo")
#define MSG_TOOL				MSG("�c�[��", "Tool")
#define MSG_EDIT				MSG("�ҏW", "Edit")
#define MSG_INITAPPBTN_FAIL		MSG("�A�v���P�[�V�����{�^���̏������Ɏ��s���܂���", "Initialize APP button failed.")
#define MSG_NEW					MSG("�V�K", "New")
#define MSG_UPDATE				MSG("�X�V", "Update")
#define MSG_RONLY				MSG("�{��", "View")
#define MSG_DEFAULT_HEADLINE	MSG("����", "memo")
#define MSG_SAVE_FAILED			MSG("�����̕ۑ��Ɏ��s���܂���(%d)", "Save memo failed(%d).")
#define MSG_GETSIPSTAT_FAILED	MSG("SIP�̏�Ԏ擾�Ɏ��s���܂���", "Get SIP status failed.")
#define MSG_CREATEFOLDER_FAILED MSG("�t�H���_�̍쐬�Ɏ��s���܂���", "Create folder failed.")
#define MSG_ERASE_PW			MSG("�L�����Ă����p�X���[�h���������܂���", "Erase password in memory")
#define MSG_ERASE_PW_TITLE		MSG("�p�X���[�h����", "Erase password")
#define MSG_FOLDING_FAILED		MSG("�܂�Ԃ������̐؂�ւ��Ɏ��s���܂���", "Folding failed.")
#define MSG_NOT_ENOUGH_MEMORY	MSG("�������̊m�ۂɎ��s���܂���", "Not enough memory.")
#define MSG_PASS_NOT_MATCH		MSG("�p�X���[�h����v���܂���", "Password is not match.")
#define MSG_CANT_GET_PASS		MSG("�p�X���[�h���擾�ł��܂���ł���", "Can't get password.")
#define MSG_TIMER_SET_FAILED	MSG("�p�X���[�h�^�C�}�[���ݒ�ł��܂���ł���", "Set timer failed.")
#define MSG_RMDIR_FAILED		MSG("�f�B���N�g���폜�Ɏ��s���܂���", "Remove directory failed.")
#define MSG_OTHER_FILE_EXISTS	MSG("�����ȊO�̃t�@�C�������݂��܂�", "There are file that is not memo.")
#define MSG_RMFILE_FAILED		MSG("�t�@�C���폜�Ɏ��s���܂���", "Remove file failed.")
#define MSG_MKDIR_FAILED		MSG("�f�B���N�g���쐬�Ɏ��s���܂���", "Create directory failed.")
#define MSG_FILECOPY_FAILED		MSG("�t�@�C���̃R�s�[�Ɏ��s���܂���", "Copy file failed.")
#define MSG_MEMO_EDITED			MSG("�����͕ҏW����Ă��܂��B�ۑ����܂���?", "Memo has been modified. Save changes?")
#define MSG_CONFIRM_SAVE		MSG("�ۑ��̊m�F", "Confirm")
#define MSG_CANT_OPEN_MEMO		MSG("�����͊J���܂���ł���", "Can't open memo")
#define MSG_PASS_NOT_MATCH2		MSG("���͂��ꂽ�p�X���[�h���o�^���ꂽ����ƈ�v���܂���B���s���܂���?", "Password is not match. Continue?")
#define MSG_PASS_NOT_MATCH3		MSG("���͂��ꂽ�p�X���[�h���o�^���ꂽ����ƈ�v���܂���B", "Password is not match.")
#define MSG_PASS_MISMATCH_TTL	MSG("�p�X���[�h�s��v", "Password mismatch")
#define MSG_IS_CREATE_DIR		MSG("�f�B���N�g�������݂��܂���B�쐬���܂���?", "Directory not exists. Create?")
#define MSG_CREATE_DIR_TTL		MSG("�����f�B���N�g���̍쐬", "Create directory")
#define MSG_CHOOSE_FOLDER		MSG("�t�H���_�̑I��", "Choose folder")
#define MSG_REG_PASSWD			MSG("�p�X���[�h��o�^���܂��B�ύX�͑����ɔ��f����܂��B��낵���ł���?", "Register password.")
#define MSG_REG_PASSWD_TTL		MSG("�p�X���[�h�̕ύX", "Change password")
#define MSG_PLEASE_SET_PASS		MSG("�p�X���[�h��ݒ肵�Ă�������", "Please set password.")
#define MSG_GET_PASS_FAILED		MSG("�p�X���[�h�̎擾�Ɏ��s���܂���", "Get password failed.")
#define MSG_GET_FP_FAILED		MSG("�t�B���K�[�v�����g�̎擾�Ɏ��s���܂���", "Get fingerprint failed.")
#define MSG_PW_REGED			MSG("�p�X���[�h���o�^����܂���", "Password has registered.")
#define MSG_PW_REG_FAILED		MSG("�p�X���[�h�̓o�^�Ɏ��s���܂���", "Register password failed.")
#define MSG_SAVE_DATA_FAILED	MSG("�f�[�^�̕ۑ��Ɏ��s���܂���", "Save data failed.")
#define MSG_ENCRYPTION_FAILED	MSG("�Í����Ɏ��s���܂���(%d)", "Encryption failed.(%d)")
#define MSG_DECRYPTION_FAILED	MSG("�������Ɏ��s���܂���(%d)", "Decryption faield.(%d)")
#define MSG_MOVE_MEMO_FAILED	MSG("�ړ��Ɏ��s���܂���", "Move failed.")
#define MSG_COPY_MEMO_FAILED	MSG("�R�s�[�Ɏ��s���܂���", "Copy failed.")
#define MSG_STRING_NOT_FOUND	MSG("������͌�����܂���ł����B", "String not found")
#define MSG_CONFIRM_DELETE		MSG("�I�������������폜���܂��B��낵���ł����H", "Delete selected memo?")
#define MSG_DELETE_TTL			MSG("�����̍폜", "Delete memo")
#define MSG_DELETE_FAILED		MSG("�����̍폜�Ɏ��s���܂���", "Delete memo failed.")
#define MSG_DELETE_PREV_CRYPT_MEMO_FAILED \
								MSG("�Í����O�̃����̍폜�Ɏ��s���܂����B�蓮�ŏ����Ă�������",\
									"Delete memo that before encrypted failed. Please delete manually.")
#define MSG_DEL_PREV_DECRYPT_MEMO_FAILED \
								MSG("�������O�̃����̍폜�Ɏ��s���܂����B�蓮�ŏ����Ă�������",\
									"Delete memo that before decrypted failed. Please delete manually.")
#define MSG_NO_FILENAME			MSG("�t�@�C�������w�肳��Ă��܂���", "No filename.")
#define MSG_SAME_FILE			MSG("�����̃t�@�C�������݂��܂�", "Same name file exists.")
#define MSG_RENAME_FAILED		MSG("�t�@�C�����̕ύX�Ɏ��s���܂���: %d", "Rename file failed(%d).")
#define MSG_CONFIRM_DEL_FOLDER	MSG("�I�������t�H���_���폜���܂��B��낵���ł����H", "Delete selected folder?")
#define MSG_DEL_FOLDER_TTL		MSG("�t�H���_�̍폜", "Delete folder")
#define MSG_NO_FOLDERNAME		MSG("�t�H���_�����w�肳��Ă��܂���", "No foldername.")
#define MSG_SAME_FOLDER			MSG("�����̃t�H���_�����݂��܂�", "Same folder name exists.")
#define MSG_REN_FOLDER_FAILED	MSG("�t�H���_���̕ύX�Ɏ��s���܂���: %d", "Rename folder failed(%d).")
#define MSG_GET_DATE_FAILED		MSG("���t�̎擾�Ɏ��s���܂���", "Get dateformat failed.")
#define MSG_ROOT_NOT_ALLOWED	MSG("\\�̓����t�H���_�Ƃ��Ďw��ł��܂���", "Can't set memo folder as '\\'")
#define MSG_DATEFORMAT_DESC		MSG("%y\t�F�N4��(2001)\r\n%Y\t�F�N��2��(01)\r\n%M\t�F��(01,02,...)\r\n%b\t�F��(Jan,Feb..)\r\n%d\t�F��(01,02..)\r\n%D\t�F��(1,2,..)\r\n%w\t�F�j��(��,��..)\r\n%W\t�F�j��(Sun,Mon)\r\n%H\t�F��(12H)\r\n%h\t�F��(24H)\r\n%a\t�FAM,PM\r\n%m\t�F��(01,02..)\r\n%s\t�F�b(01,02..)\r\n%%\t�F\"%\"\r\n%n\t�F���s", \
									"%y\t:Year(2001)\r\n%Y\t:Year(01)\r\n%M\t:Month(01,02,..)\r\n%b\t:Month(Jan,Feb,..)\r\n%d\t:Day(01,02..)\r\n%D\t:Day(1,2,..)\r\n%W\t:WDay(Sun,Mon,..)\r\n%H\t:Hour(12H)\r\n%h\t:Hour(24H)\r\n%a\t:AM,PM\r\n%m\t:Min(01,02,..)\r\n%s\t:Sec(01,02,..)\r\n%%\t:\"%\"\r\n%n\t:newline")
#define MSG_VIRTUAL_FOLDER		MSG("��������", "Search result")
#define MSG_CONFIRM_ENCRYPT_FOLDER \
								MSG("�t�H���_�̃t�@�C���ɑ΂��ĈÍ������s���܂��B��낵���ł����H", "Encrypt files under the folder?")
#define MSG_CONFIRM_ENCRYPT_FOLDER_TTL \
								MSG("�Í���", "Encrypt files")
#define MSG_PLAIN_TEXT_DEL_FAILED \
								MSG("�������̃t�@�C���ɂ��ĈÍ����O�̃t�@�C���̍폜�Ɏ��s���܂���", "Delete some plain file failed.")
#define MSG_ENCRYPT_FAILED		MSG("�������̃t�@�C���ɂ��ĈÍ����Ɏ��s���܂���", "Some files are not encrypted.")
#define MSG_CONFIRM_DECRYPT_FOLDER \
								MSG("�t�H���_�̃t�@�C���ɑ΂��ĕ��������s���܂��B��낵���ł����H", "Decrypt files under the folder?")
#define MSG_CONFIRM_DECRYPT_FOLDER_TTL \
								MSG("������", "Decrypt files")
#define MSG_CRYPT_FILE_DEL_FAILED \
								MSG("�������̃t�@�C���ɂ��ĈÍ��t�@�C���̍폜�Ɏ��s���܂���", "Delete some encrypted file failed.")
#define MSG_DECRYPT_FAILED		MSG("�������̃t�@�C���ɂ��ĕ������Ɏ��s���܂���", "Some files are not decrypted.")
#define MSG_CHECKBF_FAILED		MSG("�Í������C�u�����̃Z���t�e�X�g�Ɏ��s���܂���(%s)�B�Í���������ɍs���Ȃ��\��������܂�", "Self test of encryption libs failed(%s). Encryption are not work correctly.")
#define MSG_CHECKBF_TTL			MSG("�Z���t�e�X�g���s", "Self test failed")
#define MSG_SEARCH_CANCELING	MSG("�L�����Z�����Ă��܂�...", "Wait a moment...")
#define MSG_SEARCH_FAILED		MSG("�����Ɏ��s���܂���(%d)", "Search failed(%d).")
#define MSG_STRING_SEARCH_CANCELED \
								MSG("�����̓L�����Z������܂���", "Search has been canceled.")

#define MSG_GREPTAB_MAIN_TTL	MSG("�N�C�b�N�t�B���^", "Quick filter")
#define MSG_INSERTVFOLDER_FAIL	MSG("���z�t�H���_�̍쐬�Ɏ��s���܂���", "Create virtual folder failed.")
#define MSG_GREP_NONAME_LABEL	MSG("��������", "Grep result")
#define MSG_VFOLDER_NAME_TTL	MSG("���z�t�H���_��", "Virtual folder name")
#define MSG_CONFIRM_DEL_VFOLDER	MSG("�I���������z�t�H���_���폜���܂��B��낵���ł����H", "Delete selected virtual folder?")
#define MSG_DEL_VFOLDER_TTL		MSG("���z�t�H���_�̍폜", "Delete virtual folder")
#define MSG_FILTERDEFDLG_TTL	MSG("�t�B���^�̐ݒ�", "Filter settings")
#define MSG_FILTERDEFPROPTTL_SRC \
								MSG("�\�[�X", "Source")
#define MSG_FILTERDEFPROPTTL_FILTER \
								MSG("�t�B���^", "Filter")
#define MSG_FILTERDEFPROPTTL_FILTERTYPE_HDR \
								MSG("�t�B���^���", "Filter ype")
#define MSG_FILTERDEFPROPTTL_FILTERVAL_HDR \
								MSG("����", "Condition")
#define MSG_STREAM_NAME_REGEXP	MSG("����", "Find")
#define MSG_STREAM_NAME_LIMIT	MSG("�A�C�e����", "Num of items")
#define MSG_STREAM_NAME_TIMESTAMP \
								MSG("�ŏI�X�V��", "Last update")
#define MSG_STREAM_NAME_SORT	MSG("�\�[�g", "Sort")
#define MSG_STREAM_VALUE_LIMIT	MSG("%d��", "%d item(s)")
#define MSG_STREAM_VALUE_TIMESTAMP_NEWER \
								MSG("%d���O���V��������", "Newer than %d day before")
#define MSG_STREAM_VALUE_TIMESTAMP_OLDER \
								MSG("%d���O���Â�����", "Older than %d day before")
#define MSG_STREAM_VALUE_SORT_FNAME_ASC \
								MSG("�t�@�C�����̎�����", "Dictionary order")
#define MSG_STREAM_VALUE_SORT_FNAME_DSC \
								MSG("�t�@�C�����̋t������", "Reverse dic order")
#define MSG_STREAM_VALUE_SORT_LASTUPD_ASC \
								MSG("�ŏI�X�V���̌Â���", "Old order of last update")
#define MSG_STREAM_VALUE_SORT_LASTUPD_DSC \
								MSG("�ŏI�X�V���̐V������", "New order of last update")
#define MSG_DLG_INIT_FAIL		MSG("�_�C�A���O�̏������Ɏ��s���܂���", "Initialize dialog failed.")
#define MSG_INVALID_REGEXP		MSG("���K�\���̐����Ɏ��s���܂���", "Generate pattern failed")
#define MSG_SET_POSITIVE	MSG("0�ȏ�̐����w�肵�Ă�������", "Please set positive number")
#define MSG_INVALID_FONT_SIZE	MSG("�t�H���g�T�C�Y���s���ł�", "Invalid font size")
#define MSG_DST_FOLDER_IS_SRC_SUBFOLDER \
								MSG("�󂯑��̃t�H���_�͑��葤�t�H���_�̃T�u�t�H���_�ł��B", "Dst folder is subfolder of src folder")
#endif

