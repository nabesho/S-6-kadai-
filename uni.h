/** uni.h *****************************************************

NOTES :
1. UNIのfield名が２文字以上に対応
2. 空行があっても正常に読み込める
3. 絶対位置(position)を採用
4. load_uni 関数実装
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 読み込み可能なunieventの最大数 */
#define MAX_UNIEVENTS 1024

#define FLAG_ON 1
#define FLAG_OFF 0


/** global 変数定義 ***************************************/

// unievent構造体定義
struct SUniEvent{
	long interval;   // ^I  インターバル
	int key_number;    // ^K  音高
	int velocity;   // ^V  ベロシティー（音の強さ）
	int length;     // ^L  音の長さ
	int channel;    // ^W  声部
	int measure;    // ^M  小節
	int tempo;      // ^T  テンポ

	long position; // 曲開始からの位置（Σinterval）
};

// unieventの配列
struct SUniEvent unievent[MAX_UNIEVENTS + 1];

// unievent[]が保持しているEventの数
int n_unievents = 0;


/** init_unievents() : unievent[]の初期化用関数の定義 **************/
void init_unievents()
{
	int i;

	for (i = 0; i < MAX_UNIEVENTS ; i++){
		unievent[i].interval = 0;
		unievent[i].key_number = -1;
		unievent[i].velocity = 0;
		unievent[i].length = 0;
		unievent[i].channel = 1;
		unievent[i].measure = 0;
		unievent[i].tempo = 0;

		unievent[i].position = 0;
	}

	n_unievents = 0;
	return;
}



/*** unievent[].interval からunievent[].position を計算する関数の定義 ******/
void calc_uni_position(){

	int i;
	unievent[0].position = 0;
	for (i = 1; i <= n_unievents ; i++){
		unievent[i].position
			= unievent[i].interval + unievent[i-1].position ;
	}
	return ;
}



/*** display_unievents() :  unievent[] 全要素表示する関数の定義 ************/
void display_unievents(){

	int i;
	calc_uni_position(); // position（絶対時間値）の計算
	for ( i = 1; i <= n_unievents ; i++){
		printf("%4d : [pos:%5d] ^I%4d ^K%3d ^V%3d ^L%4d ^W%2d ^M%3d ^T%3d\n",
			i,
			unievent[i].position,
			unievent[i].interval,
			unievent[i].key_number,
			unievent[i].velocity,
			unievent[i].length,
			unievent[i].channel,
			unievent[i].measure,
			unievent[i].tempo
		);
	}
}


/*** int set_unievent_value()  ****************************************
   n 番目のunieventの指定したフィールドにvalueを設定

引数
  int n : n 番目のunievent
  const char fieldname[] : フィールドの名前
  int value : 設定値

返り値
　正常に値を設定できれば1、設定できなければ0を返す

**********************************************************************/
int set_unievent_value(int n, const char *fieldname, int value ){

	/* n が n_unievents よりも大きい時は、
	　 unievent[]の最後に要素を一つ加える */
	if (n > n_unievents){
		n_unievents++;
		printf("unievent数を %d に更新しました。\n",n_unievents);
        n = n_unievents;
	}

	/* 文字とマッチしたメンバ変数に値を代入 */
	if     (strcmp(fieldname,"I") == 0){
		unievent[n].interval = (long)value;
	}
	else if(strcmp(fieldname,"K") == 0){
		unievent[n].key_number = value;
	}
	else if(strcmp(fieldname,"V") == 0){
		unievent[n].velocity = value;
	}
	else if(strcmp(fieldname,"L") == 0){
		unievent[n].length = value;
	}
	else if(strcmp(fieldname,"W") == 0){
		unievent[n].channel = value;
	}
	else if(strcmp(fieldname,"M") == 0){
		unievent[n].measure = value;
	}
	else if(strcmp(fieldname,"T") == 0){
		unievent[n].tempo = value;
	}
	else {
		printf("%s は未定義のフィールドです。\n",fieldname);
		return 0; //  value set 失敗
	}

	//printf("Set unievent[%d].%s = %d\n",n, fieldname, value);

	return 1;  // value set 成功
}


/*** load_uni() *****************************************************
NOTE:  指定したUNIファイルを開き、unievent[]に音符情報を格納
 引数 : 読み込むUNIファイルの名前
 返り値 : 正常に読み込めれば1、読み込めなければ0
**********************************************************************/
int load_uni(const char *unifilename)
{
	/* ==== 変数宣言部 ======================================== */
	FILE *fp;

	char ch;  // UNIファイルから読み込む１文字を格納
	int number;  // UNIのフィールド値の数字１字を格納

	char fieldname[10];   // fieldname を一時的に格納
	int i;                // fieldname の文字数カウント用
	int value = 0;        // field値を一時的に格納

	/* UNIファイル読込み中のデータ識別の為のフラグ */
	int f_fieldname = FLAG_OFF;  // fieldnameのフラグ
	int f_value = FLAG_OFF;      // valueのフラグ

	/* ======================================================== */

	init_unievents();   // unievent[] 初期化
	fp = fopen(unifilename, "r");      // UNIfileを開く
	i = 0;              // fieldname の文字数カウント用

	/** ↓ UNIデータ読み込みルーチン ***********************/
	while( (ch = fgetc(fp)) != EOF ){

		switch (ch)
		{
		case '^':  // field名の読込み開始記号 ==========================/

			if ( (!f_fieldname) && (!f_value) ){
				if(n_unievents >= MAX_UNIEVENTS){
					printf("UNIファイルロード中に、UNIイベント数の上限を超えました。\n");
					return 0; // エラー終了
				}
				n_unievents++;
			}
			/*** valueフラグをOFF. 直前に読み込んだvalueをセット ******/
				if(f_value == FLAG_ON){
					f_value = FLAG_OFF;
					set_unievent_value(n_unievents, fieldname, value);
				}
			/*** fieldnameフラグをON. valueの読込みの準備 *************/
				f_fieldname = FLAG_ON ;
				i = 0;	// fieldname 読み込み用カウンタリセット
				break;

		case '%':  /*== field値の読込み開始記号 ============================*/
			/***** fieldnameフラグをOFF. field名はここまで ****************/
				f_fieldname = FLAG_OFF;
				fieldname[i] = '\0';  // fieldnameを閉じる
			/***** valueフラグをON. fieldname の読込みの準備 **************/
				f_value = FLAG_ON ;
				value = 0; // field値の初期化
				break;

		case '\n':   /*== 改行コード。１つのunieventの区切り ===============*/
			/****** valueフラグがONなら、直前に読み込んだvalueをセット *****/
				if(f_value == FLAG_ON){
					set_unievent_value(n_unievents, fieldname, value);
					f_value = FLAG_OFF;
				}
				break;

		default:	// === 文字が ^ % \n 以外の時 ============================/
			// fieldnameフラグがONならfieldnameとして読み込む --------------
			if ( f_fieldname && (! f_value) )
			{
				fieldname[i] = ch;
				i++;
			}
			// valueフラグがONならvalueとして読み込む ----------------------
			else if ( (! f_fieldname) && f_value)
			{
				number = ch - '0';  // 数字(char)を数値(int)に変換

				// 読み込んだ文字が数字でなければエラー
				if ( number < 0 || number > 9 ){
					printf("\nfield値に文字 %c が含まれていました。\n",ch);
					printf("UNIデータのfield値には数字以外記述しないでください。\n");
					return 0;   // エラー終了
				}
				value = value * 10 + number ;  // value値に数値を１桁追加
			}
			 // fieldnameもvalueもフラグが立っていない or 両方立っている場合は不正----
			else{
				printf("UNIファイルのフォーマットが不正です。\n");
				printf("１行の始めは必ず ^フィールド名 にしてください。\n");
				return 0;  // エラー終了
			}
			break;
		}

	}

	// 最後に改行が無かった為に読み残したデータを格納
	if(f_value == FLAG_ON){
		f_value = FLAG_OFF;
		set_unievent_value(n_unievents, fieldname, value);
	}
    /*** ↑ UNIデータ読み込み終了 ********************/

	calc_uni_position();  // unieventの絶対時間(position)を計算

	fclose(fp);	  // UNIファイルを閉じる
	return 1;     // 正常終了。1を返す
}


/*** EOF *********************************************************************/
