#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef TOMBO_LANG_ENGLISH
#define MSG(J,E) TEXT(E)
#else
#define MSG(J,E) TEXT(J)
#endif

#define MSG_TOOLTIPS_NEWMEMO	MSG("新規メモ", "New Memo")
#define MSG_TOOLTIPS_RETURNLIST	MSG("一覧に戻る", "Return to list")
#define MSG_TOOLTIPS_SAVE		MSG("保存", "Save")
#define MSG_TOOLTIPS_INSDATE1	MSG("日付1の挿入", "Insert date1")
#define MSG_TOOLTIPS_INSDATE2	MSG("日付2の挿入", "Insert date2")
#define MSG_MEMO                MSG("メモ", "Memo")
#define MSG_TOOL				MSG("ツール", "Tool")
#define MSG_EDIT				MSG("編集", "Edit")
#define MSG_INITAPPBTN_FAIL		MSG("アプリケーションボタンの初期化に失敗しました", "Initialize APP button failed.")
#define MSG_NEW					MSG("新規", "New")
#define MSG_UPDATE				MSG("更新", "Update")
#define MSG_RONLY				MSG("閲覧", "View")
#define MSG_DEFAULT_HEADLINE	MSG("無題", "memo")
#define MSG_SAVE_FAILED			MSG("メモの保存に失敗しました(%d)", "Save memo failed(%d).")
#define MSG_GETSIPSTAT_FAILED	MSG("SIPの状態取得に失敗しました", "Get SIP status failed.")
#define MSG_CREATEFOLDER_FAILED MSG("フォルダの作成に失敗しました", "Create folder failed.")
#define MSG_ERASE_PW			MSG("記憶していたパスワードを消去しました", "Erase password in memory")
#define MSG_ERASE_PW_TITLE		MSG("パスワード消去", "Erase password")
#define MSG_FOLDING_FAILED		MSG("折り返し処理の切り替えに失敗しました", "Folding failed.")
#define MSG_NOT_ENOUGH_MEMORY	MSG("メモリの確保に失敗しました", "Not enough memory.")
#define MSG_PASS_NOT_MATCH		MSG("パスワードが一致しません", "Password is not match.")
#define MSG_CANT_GET_PASS		MSG("パスワードが取得できませんでした", "Can't get password.")
#define MSG_TIMER_SET_FAILED	MSG("パスワードタイマーが設定できませんでした", "Set timer failed.")
#define MSG_RMDIR_FAILED		MSG("ディレクトリ削除に失敗しました", "Remove directory failed.")
#define MSG_OTHER_FILE_EXISTS	MSG("メモ以外のファイルが存在します", "There are file that is not memo.")
#define MSG_RMFILE_FAILED		MSG("ファイル削除に失敗しました", "Remove file failed.")
#define MSG_MKDIR_FAILED		MSG("ディレクトリ作成に失敗しました", "Create directory failed.")
#define MSG_FILECOPY_FAILED		MSG("ファイルのコピーに失敗しました", "Copy file failed.")
#define MSG_MEMO_EDITED			MSG("メモは編集されています。保存しますか?", "Memo has been modified. Save changes?")
#define MSG_CONFIRM_SAVE		MSG("保存の確認", "Confirm")
#define MSG_CANT_OPEN_MEMO		MSG("メモは開けませんでした", "Can't open memo")
#define MSG_PASS_NOT_MATCH2		MSG("入力されたパスワードが登録されたそれと一致しません。続行しますか?", "Password is not match. Continue?")
#define MSG_PASS_NOT_MATCH3		MSG("入力されたパスワードが登録されたそれと一致しません。", "Password is not match.")
#define MSG_PASS_MISMATCH_TTL	MSG("パスワード不一致", "Password mismatch")
#define MSG_IS_CREATE_DIR		MSG("ディレクトリが存在しません。作成しますか?", "Directory not exists. Create?")
#define MSG_CREATE_DIR_TTL		MSG("メモディレクトリの作成", "Create directory")
#define MSG_CHOOSE_FOLDER		MSG("フォルダの選択", "Choose folder")
#define MSG_REG_PASSWD			MSG("パスワードを登録します。変更は即座に反映されます。よろしいですか?", "Register password.")
#define MSG_REG_PASSWD_TTL		MSG("パスワードの変更", "Change password")
#define MSG_PLEASE_SET_PASS		MSG("パスワードを設定してください", "Please set password.")
#define MSG_GET_PASS_FAILED		MSG("パスワードの取得に失敗しました", "Get password failed.")
#define MSG_GET_FP_FAILED		MSG("フィンガープリントの取得に失敗しました", "Get fingerprint failed.")
#define MSG_PW_REGED			MSG("パスワードが登録されました", "Password has registered.")
#define MSG_PW_REG_FAILED		MSG("パスワードの登録に失敗しました", "Register password failed.")
#define MSG_SAVE_DATA_FAILED	MSG("データの保存に失敗しました", "Save data failed.")
#define MSG_ENCRYPTION_FAILED	MSG("暗号化に失敗しました(%d)", "Encryption failed.(%d)")
#define MSG_DECRYPTION_FAILED	MSG("復号化に失敗しました(%d)", "Decryption faield.(%d)")
#define MSG_MOVE_MEMO_FAILED	MSG("移動に失敗しました", "Move failed.")
#define MSG_COPY_MEMO_FAILED	MSG("コピーに失敗しました", "Copy failed.")
#define MSG_STRING_NOT_FOUND	MSG("文字列は見つかりませんでした。", "String not found")
#define MSG_CONFIRM_DELETE		MSG("選択したメモを削除します。よろしいですか？", "Delete selected memo?")
#define MSG_DELETE_TTL			MSG("メモの削除", "Delete memo")
#define MSG_DELETE_FAILED		MSG("メモの削除に失敗しました", "Delete memo failed.")
#define MSG_DELETE_PREV_CRYPT_MEMO_FAILED \
								MSG("暗号化前のメモの削除に失敗しました。手動で消してください",\
									"Delete memo that before encrypted failed. Please delete manually.")
#define MSG_DEL_PREV_DECRYPT_MEMO_FAILED \
								MSG("復号化前のメモの削除に失敗しました。手動で消してください",\
									"Delete memo that before decrypted failed. Please delete manually.")
#define MSG_NO_FILENAME			MSG("ファイル名が指定されていません", "No filename.")
#define MSG_SAME_FILE			MSG("同名のファイルが存在します", "Same name file exists.")
#define MSG_RENAME_FAILED		MSG("ファイル名の変更に失敗しました: %d", "Rename file failed(%d).")
#define MSG_CONFIRM_DEL_FOLDER	MSG("選択したフォルダを削除します。よろしいですか？", "Delete selected folder?")
#define MSG_DEL_FOLDER_TTL		MSG("フォルダの削除", "Delete folder")
#define MSG_NO_FOLDERNAME		MSG("フォルダ名が指定されていません", "No foldername.")
#define MSG_SAME_FOLDER			MSG("同名のフォルダが存在します", "Same folder name exists.")
#define MSG_REN_FOLDER_FAILED	MSG("フォルダ名の変更に失敗しました: %d", "Rename folder failed(%d).")
#define MSG_GET_DATE_FAILED		MSG("日付の取得に失敗しました", "Get dateformat failed.")
#define MSG_ROOT_NOT_ALLOWED	MSG("\\はメモフォルダとして指定できません", "Can't set memo folder as '\\'")
#define MSG_DATEFORMAT_DESC		MSG("%y\t：年4桁(2001)\r\n%Y\t：年下2桁(01)\r\n%M\t：月(01,02,...)\r\n%b\t：月(Jan,Feb..)\r\n%d\t：日(01,02..)\r\n%D\t：日(1,2,..)\r\n%w\t：曜日(日,月..)\r\n%W\t：曜日(Sun,Mon)\r\n%H\t：時(12H)\r\n%h\t：時(24H)\r\n%a\t：AM,PM\r\n%m\t：分(01,02..)\r\n%s\t：秒(01,02..)\r\n%%\t：\"%\"\r\n%n\t：改行", \
									"%y\t:Year(2001)\r\n%Y\t:Year(01)\r\n%M\t:Month(01,02,..)\r\n%b\t:Month(Jan,Feb,..)\r\n%d\t:Day(01,02..)\r\n%D\t:Day(1,2,..)\r\n%W\t:WDay(Sun,Mon,..)\r\n%H\t:Hour(12H)\r\n%h\t:Hour(24H)\r\n%a\t:AM,PM\r\n%m\t:Min(01,02,..)\r\n%s\t:Sec(01,02,..)\r\n%%\t:\"%\"\r\n%n\t:newline")
#define MSG_VIRTUAL_FOLDER		MSG("検索結果", "Search result")
#define MSG_CONFIRM_ENCRYPT_FOLDER \
								MSG("フォルダのファイルに対して暗号化を行います。よろしいですか？", "Encrypt files under the folder?")
#define MSG_CONFIRM_ENCRYPT_FOLDER_TTL \
								MSG("暗号化", "Encrypt files")
#define MSG_PLAIN_TEXT_DEL_FAILED \
								MSG("いくつかのファイルについて暗号化前のファイルの削除に失敗しました", "Delete some plain file failed.")
#define MSG_ENCRYPT_FAILED		MSG("いくつかのファイルについて暗号化に失敗しました", "Some files are not encrypted.")
#define MSG_CONFIRM_DECRYPT_FOLDER \
								MSG("フォルダのファイルに対して復号化を行います。よろしいですか？", "Decrypt files under the folder?")
#define MSG_CONFIRM_DECRYPT_FOLDER_TTL \
								MSG("復号化", "Decrypt files")
#define MSG_CRYPT_FILE_DEL_FAILED \
								MSG("いくつかのファイルについて暗号ファイルの削除に失敗しました", "Delete some encrypted file failed.")
#define MSG_DECRYPT_FAILED		MSG("いくつかのファイルについて復号化に失敗しました", "Some files are not decrypted.")
#define MSG_CHECKBF_FAILED		MSG("暗号化ライブラリのセルフテストに失敗しました(%s)。暗号化が正常に行われない可能性があります", "Self test of encryption libs failed(%s). Encryption are not work correctly.")
#define MSG_CHECKBF_TTL			MSG("セルフテスト失敗", "Self test failed")
#define MSG_SEARCH_CANCELING	MSG("キャンセルしています...", "Wait a moment...")
#define MSG_SEARCH_FAILED		MSG("検索に失敗しました(%d)", "Search failed(%d).")
#define MSG_STRING_SEARCH_CANCELED \
								MSG("検索はキャンセルされました", "Search has been canceled.")

#define MSG_GREPTAB_MAIN_TTL	MSG("クイックフィルタ", "Quick filter")
#define MSG_INSERTVFOLDER_FAIL	MSG("仮想フォルダの作成に失敗しました", "Create virtual folder failed.")
#define MSG_GREP_NONAME_LABEL	MSG("検索結果", "Grep result")
#define MSG_VFOLDER_NAME_TTL	MSG("仮想フォルダ名", "Virtual folder name")
#define MSG_CONFIRM_DEL_VFOLDER	MSG("選択した仮想フォルダを削除します。よろしいですか？", "Delete selected virtual folder?")
#define MSG_DEL_VFOLDER_TTL		MSG("仮想フォルダの削除", "Delete virtual folder")
#define MSG_FILTERDEFDLG_TTL	MSG("フィルタの設定", "Filter settings")
#define MSG_FILTERDEFPROPTTL_SRC \
								MSG("ソース", "Source")
#define MSG_FILTERDEFPROPTTL_FILTER \
								MSG("フィルタ", "Filter")
#define MSG_FILTERDEFPROPTTL_FILTERTYPE_HDR \
								MSG("フィルタ種別", "Filter ype")
#define MSG_FILTERDEFPROPTTL_FILTERVAL_HDR \
								MSG("条件", "Condition")
#define MSG_STREAM_NAME_REGEXP	MSG("検索", "Find")
#define MSG_STREAM_NAME_LIMIT	MSG("アイテム数", "Num of items")
#define MSG_STREAM_NAME_TIMESTAMP \
								MSG("最終更新日", "Last update")
#define MSG_STREAM_NAME_SORT	MSG("ソート", "Sort")
#define MSG_STREAM_VALUE_LIMIT	MSG("%d件", "%d item(s)")
#define MSG_STREAM_VALUE_TIMESTAMP_NEWER \
								MSG("%d日前より新しいもの", "Newer than %d day before")
#define MSG_STREAM_VALUE_TIMESTAMP_OLDER \
								MSG("%d日前より古いもの", "Older than %d day before")
#define MSG_STREAM_VALUE_SORT_FNAME_ASC \
								MSG("ファイル名の辞書順", "Dictionary order")
#define MSG_STREAM_VALUE_SORT_FNAME_DSC \
								MSG("ファイル名の逆辞書順", "Reverse dic order")
#define MSG_STREAM_VALUE_SORT_LASTUPD_ASC \
								MSG("最終更新日の古い順", "Old order of last update")
#define MSG_STREAM_VALUE_SORT_LASTUPD_DSC \
								MSG("最終更新日の新しい順", "New order of last update")
#define MSG_DLG_INIT_FAIL		MSG("ダイアログの初期化に失敗しました", "Initialize dialog failed.")
#define MSG_INVALID_REGEXP		MSG("正規表現の生成に失敗しました", "Generate pattern failed")
#define MSG_SET_POSITIVE	MSG("0以上の数を指定してください", "Please set positive number")
#define MSG_INVALID_FONT_SIZE	MSG("フォントサイズが不正です", "Invalid font size")
#define MSG_DST_FOLDER_IS_SRC_SUBFOLDER \
								MSG("受け側のフォルダは送り側フォルダのサブフォルダです。", "Dst folder is subfolder of src folder")
#endif

