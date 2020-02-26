/*** unifilter.c *****************************************
 NOTE :
**********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "uni.h"

void MoveKey(int start, int end, int value){ //移調 動作確認済み
	if (value * value > 144){
		printf("移調幅が不適です。");
		exit(-1);
	}
	int i,j;
	for (i = start; i <= end; i++){
		if (unievent[i].key_number != -1){
			j = unievent[i].key_number + value;
			//移調先が鍵盤の外だった場合にオク下げ、オク上げを行う
			if (j < 0){
				j += 12;
			}
			else if (j>127){
				j -= 12;
			}
			unievent[i].key_number = j;
		}
	}

	return;
}

void CVelocity( int start, int end, int value){ //強弱変更 動作確認済み
	if (value <10 || value > 1000){
		printf("強弱変更幅が不適です。");
		exit(-1);
	}

	int i;
	for (i = start; i <= end; i++){
		unievent[i].velocity = unievent[i].velocity *value / 100;
	}
	return;
}

void CTempo(int start, int end, int value){ //テンポ変更 動作確認済み
	if (value <10 || value > 1000){
		printf("テンポの変更幅が不適です。");
		exit(-1);
	}

	int i;
	for (i = start; i <= end; i++){
		unievent[i].length = unievent[i].length *100/value;
		unievent[i + 1].interval = unievent[i + 1].interval  *100/value;
	}
	return;
}

void Staccato(int start, int end){ //スタッカート 動作確認済み
	int i;
	for (i = start; i <= end; i++){
		unievent[i].length = unievent[i].length / 2;
	}
	return;
}

void Legato(int start, int end){ //レガート 動作確認済み
	int i;
	for (i = start; i <= end; i++){
		unievent[i].length += unievent[i].length  / 5;
	}
	return;
}

void Delete(int start, int end){ //消去 動作確認済み
	int i;
	int k = start - 1; //kはDeleteの範囲内でも残るイベントに対する処理を行うためのもの
	for (i = start; i <= end; i++){
		if (unievent[i].measure == 0){
			unievent[i].interval = 0;
			unievent[i].key_number = -1;
			unievent[i].velocity = 0;
			unievent[i].length = 0;
			unievent[i].channel = 1;
			unievent[i].position = 0;
			unievent[i].tempo = 0;
		}
		else{
			unievent[i].interval = unievent[i].position - unievent[k].position;
			k = i; //unievent[start-1]を基点として、Delete内でも残るイベントに対して、positionの差からintervalを割り当てて適切な場所に休符と小節線を作る。
		}


	}
	if (end < n_unievents){
		unievent[end + 1].interval = unievent[end + 1].position - unievent[k].position;
	}
}

void write_uni(char *filename){ //書き込み //動作確認済み

	FILE *fp;

	int i;

	if ((fp = fopen(filename, "w")) == NULL){
		exit(-1);
	}
	else{
		for (i = 1; i <= n_unievents; i++){
			if (unievent[i].key_number != -1 || unievent[i].measure != 0){ //イベントの有無の判別のためkeynumberとmeasureをフラグとして用いる
				printf("^I%%%d", unievent[i].interval);
				if (unievent[i].key_number == -1){
					printf("^M%%%d\n", unievent[i].measure);
				}
				else{
					printf("^K%%%d^V%%%d", unievent[i].key_number, unievent[i].velocity);
					printf("^L%%%d^W%%%d\n", unievent[i].length, unievent[i].channel);
				}
			}
		}

		for (i = 1; i <= n_unievents; i++){
			if (unievent[i].key_number != -1 || unievent[i].measure != 0){ //イベントの有無の判別のためkeynumberとmeasureをフラグとして用いる
				fprintf(fp, "^I%%%d", unievent[i].interval);
				if (unievent[i].key_number == -1){
					fprintf(fp, "^M%%%d\n", unievent[i].measure);
				}
				else{
					fprintf(fp, "^K%%%d^V%%%d", unievent[i].key_number, unievent[i].velocity);
					fprintf(fp, "^L%%%d^W%%%d\n", unievent[i].length, unievent[i].channel);
				}
			}
		}
		fclose(fp);
	}
}




/*** main 関数 *****************************************/

void main(int argc, char *argv[])
{
	//printf("%d",argc );
	if (argc <5){
		printf("不適切な入力です。\n");
		exit(-1);
	}
	//ファイルの読み込み
	if (!load_uni(argv[1])){
		printf("ファイルの読み込みに失敗しました。\n");
		exit(-1);
	}


	//作業を行う箇所(始まりと終わり)を読み込む
	int eventStart = (int)(strtol(argv[3],NULL,10)) ;
	int eventEnd = (int)(strtol(argv[4], NULL, 10));
	if (eventStart < 1 || eventEnd > 1024 || eventEnd - eventStart < 0){
		printf("入力された範囲が不適切です。");
	}
	//printf("start = %d,end = %d", eventStart, eventEnd);
	int value;

	//オプションによって場合分けをする
	switch (*argv[2]){
		case 'm': //移調オプション
			if (argc <6){
				printf("移調幅が未入力です。\n");
				exit(-1);
			}
			value = (int)(strtol(argv[5], NULL, 10));
			//printf("%d", value);
			MoveKey(eventStart, eventEnd, value);
			break;
		case 'v': //強弱変更オプション
			if (argc <6){
				printf("強弱の変更幅が未入力です。\n");
				exit(-1);
			}
			value = (int)(strtol(argv[5], NULL, 10));
			CVelocity(eventStart, eventEnd, value);
			break;
		case 't': //速度変更オプション
			if (argc <6){
				printf("速度の変更幅が未入力です。\n");
				exit(-1);
			}
			value = (int)(strtol(argv[5], NULL, 10));
			CTempo(eventStart, eventEnd, value);
			break;
		case 's': //スタッカートオプション
			Staccato(eventStart, eventEnd);
			break;
		case 'l': //レガートオプション
			Legato(eventStart, eventEnd);
			break;
		case 'd': //削除オプション
			Delete(eventStart, eventEnd);
			break;


	}

	write_uni(argv[1]);

	printf("動作が完了しました。\n");



	//display_unievents(); // UniEvent[]の各要素を表示
}



/*******************************************************/
