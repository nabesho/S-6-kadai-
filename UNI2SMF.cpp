#include <stdio.h>
#include <stdlib.h>
#include "uni.h"
#include "stdafx.h"

//使っている計算機がリトルエンディアンを用いているため、エンディアンの変換を行う関数を実装。
unsigned int iconv(unsigned int mm){
	unsigned int ma, m1, m2, m3, m4;
	m1 = mm % 256;
	ma = mm / 256;
	m2 = ma % 256;
	ma = ma / 256;
	m3 = ma % 256;
	m4 = ma / 256;
	ma = m1 * 256 + m2;
	ma = 256 * ma + m3;
	ma = 256 * ma + m4;

	return ma;
}

unsigned short sconv(unsigned short sm){
	unsigned short sa, s1, s2;
	s1 = sm % 256;
	s2 = sm / 256;
	sa = s1 * 256 + s2;
	return sa;
}


//write_uniは課題2で作成したものを用いた。
void write_uni(char *filename){

	FILE *fp;

	int i;

	if ((fp = fopen(filename, "w")) == NULL){
		exit(-1);
	}
	else{

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

void write_SMF(char *filename){
	int a = 0;
	int  *buf[100];
	buf[0] = &a;
	for (int i = 1; i < 10; i++){
		*(buf + 4 * i) = 0;
	}
	FILE *fp;
	int key_number, velocity, length, channel, position;
	if ((fp = fopen(filename, "wb")) == NULL){
		exit(-1);
	}


	/*
	SMFファイル内では、uniファイルやunieventにはない「鍵盤を離す」という動作がある。
	そこで、すべてのunieventに対してもう一つイベントをつくりchannelを-1倍することで「離した動作」と定義した。
	また離す動作のpositionは押す動作のposition+lengthになるのは言うまでもない。
	*/
	for (int i = 1; i <= n_unievents; i++){
		int newi = i + n_unievents;
		unievent[newi].position = unievent[i].position + unievent[i].length;
		unievent[newi].key_number = unievent[i].key_number;
		unievent[newi].channel = -1 * unievent[i].channel;
		unievent[newi].velocity = unievent[i].velocity;
	}
	int halfn = n_unievents;
		n_unievents = n_unievents + n_unievents;

		/*
		positionで線形探索して適切な順番になるようにイベントをソートした。
		*/
	for (int i = halfn + 1; i <= n_unievents; i++){
		int k = i - 1;
		while (unievent[i].position <= unievent[k].position && k>1){
			k--;
		}
		key_number = unievent[i].key_number;
		position = unievent[i].position;
		channel = unievent[i].channel;
		velocity = unievent[i].velocity;
		length = unievent[i].length;
		for (int j = i; j >= k + 2; j--){
			unievent[j].key_number = unievent[j - 1].key_number;
			unievent[j].length = unievent[j - 1].length;
			unievent[j].velocity = unievent[j - 1].velocity;
			unievent[j].channel = unievent[j - 1].channel;
			unievent[j].position = unievent[j - 1].position;
		}
		unievent[k + 1].key_number = key_number;
		unievent[k + 1].length = length;
		unievent[k + 1].velocity = velocity;
		unievent[k + 1].channel = channel;
		unievent[k + 1].position = position;
	}
	unsigned int datalength;
	unsigned long dataslength = 0;
	int n = 0;
	int temp = 0;
	/*
	SMFには小説などはサポートしない予定であったので、ノートイベント同士だけのintervalを計算した。のちに利用する。
	*/
	for (int i = 1; i <= n_unievents; i++){
		if (unievent[i].key_number >= 1){
			unievent[i].interval = unievent[i].position - unievent[n].position;
			n = i;
			datalength = unievent[i].interval;
			int k = 0;
			if (datalength == 0){
				k = 1;
			}
			while (datalength >0){
				datalength = datalength / 128;
				k++;
			}
			dataslength += (unsigned long)(3 +  k);
		}

	}

	unsigned char wchar[5] = "MThd";
	fwrite(wchar, 4, 1, fp);
	unsigned int b = iconv(6);
	unsigned int *wlong = &b;
	fwrite(wlong, 4, 1, fp);
	unsigned short c = 0;
	unsigned short *wshort = &c;
	fwrite(wshort, 2, 1, fp);
	unsigned short d = sconv(1);
	wshort = &d;
	fwrite(wshort, 2, 1, fp);
	*wshort = sconv(120);
	fwrite(wshort, 2, 1, fp);
	unsigned char wchar2[5] = "MTrk";
	fwrite(wchar2, 4, 1, fp);
	*wlong = iconv(dataslength);
	fwrite(wlong, 4, 1, fp);
	//可変長な値の格納は、適切に変数を用い、最上位のビット判定をうまく行って動作させた。
	for (int i = 1; i <= n_unievents; i++){
		if (unievent[i].key_number >= 1){
			int k = 0;
			int intv = unievent[i].interval;
			int of = intv;
			while (of > 0){
				of = of / 128;
				k++;
			}
			if (unievent[i].interval == 0){
				k = 1;
			}
			for (int j = k; j >=1; j--){
				int kk = 1;
				for (int m = 2; m <= j; m++){
					kk = kk * 128;
				}
				if (j == 1){
					*wchar = (unsigned char)(intv % 128);
				}
				else{
					*wchar = (unsigned char)((intv / kk) %128 + 128);
				}


				fwrite(wchar, 1, 1, fp);

			}
			if (unievent[i].channel > 0){
				*wchar = (16 * 9 + unievent[i].channel);
			}
			else{
				*wchar = (unsigned char)(16 * 8 - unievent[i].channel);
			}
			fwrite(wchar, 1, 1, fp);
			*wchar = (unsigned char)unievent[i].key_number;
			fwrite(wchar, 1, 1, fp);
			*wchar = (unsigned char)unievent[i].velocity;
			fwrite(wchar, 1, 1, fp);

		}
	}
	*wchar = 255;
	fwrite(wchar, 1, 1, fp);
	*wchar = 49;
	fwrite(wchar, 1, 1, fp);
	*wchar = 0;
	fwrite(wchar, 1, 1, fp);
	fclose(fp);

}


/*
SMF形式からunievents単位に落とすような関数を作成した。
煩雑になってはしまったものの、ポインタやfread関数を用いてバイナリファイルから値を取り出す作業を繰り返し
uniファイルに落とし込めるような形にすることができた。
*/
void load_SMF(char *filename){
	int z = 0;
	int *buf[100]; //いらない値を捨てるための変数
	buf[0] = &z;

	for (int i = 1; i < 10; i++){
		*(buf + 4*i) = 0;
	}

	FILE *fp;
	unsigned short aa = 0; //エンディアン変換の関係で基本的に変数はunsignedを用いた。
	unsigned short *numoftrack = &aa; //トラック数を格納する変数
	unsigned short ab = 0;
	unsigned short *timebase = &ab;//タイムベースを格納する変数

	if ((fp = fopen(filename, "rb")) == NULL){
		printf("ファイルの読み込みに失敗しました。\n");
		exit(-1);
	}
	fread(buf, 10, 1, fp);
	fread(numoftrack, 2, 1, fp);
	*numoftrack = sconv(*numoftrack); //適宜エンディアン変換を行い、ほしい値を抽出した

	fread(timebase, 2, 1, fp);
	*timebase = sconv(*timebase);
	unsigned int datalength, datanum, eventnum, keyevent[16][256], velocity, pos, channel, onoff, keynum;
	/*
	datalength:データ長
	datanum:書き込んでいる場所が何バイト目かを表す
	velocity,pos,channel,keynumはノートのステータス
	onoffは0でプッシュ、1でプール、2でそれ以外の動作を表す。
	keyeventは後述する。
	*/
	eventnum = 1;
	for (int i = 0; i < 16; i++){
		for (int j = 0; j < 256; j++){
			keyevent[i][j] = 0;
		}
	}
	pos = 0;

	for (int i = 1; i <= *numoftrack; i++){
		unsigned long deltatime; //デルタタイムを表す
		unsigned short tempdelta; //可変長を実装するために用いる。
		fread(buf, 3, 1, fp);
		unsigned char dd = 0;
		unsigned char *ck = &dd;
		fread(ck, 1, 1, fp);

		unsigned int ac = 0;
		unsigned int *dl = &ac;
		fread(dl, 4, 1, fp);

		datalength = iconv(*dl);
		datanum = 1;
		/*
		可変長の値から本来の値を取り出すため、1バイトずつ読み込んでtempdeltaに格納し、
		deltatimeを128倍しつつtempdeltaが127以下になるまで足すことを繰り返すという方法をとった。
		*/
		while (datanum <= datalength){
			deltatime = 0;
			while (1){
				deltatime = deltatime * 128;
				unsigned char ad = 0;
				unsigned char *td = &ad;
				fread(td, 1, 1, fp);
				tempdelta = (unsigned int)*td;
				datanum += 1;
				deltatime += (unsigned int)(tempdelta % 128);
				if (tempdelta < 128){
					break;
				}
			}
			pos += deltatime; //posはΣdeltatime、positionと働きが似ている。
			unsigned char ae = 0;
			unsigned char *cnl = &ae;
			fread(cnl, 1, 1, fp);
			datanum += 1;
			channel = (unsigned int)*cnl;
			//channelは値の大きさによって評価を行い、ほしい値とonoffの識別に分けた。
			if (channel <= 144){
				channel -= 128;
				onoff = 0;
			}
			else if (channel <= 160){
				channel -= 144;
				onoff = 1;
			}
			else{
				onoff = 2;
			}
			unsigned short af = 0;
			unsigned short *kn = &af;
			fread(kn, 1, 1, fp);
			datanum += 1;
			keynum = (int)*kn;
			unsigned short ag = 0;
			unsigned short *vc = &ag;
			fread(vc,1 , 1, fp);
			datanum += 1;
			velocity = (unsigned int)*vc;
			/*
			鍵盤を押したり離したりする、という作業をlengthとして取り込む。押した後は絶対離す作業が来るはず。
			ということで、16チャンネル256キー分の配列を用意する。
			あるチャンネルのキーが押されたら、そのチャンネルとキーに対応する箇所に、押されたイベント番号を格納する。
			次にそのチャンネルのキーが離されたら、格納した値からlengthを計算し、格納していた箇所の値は0に戻す。
			以上のようなアルゴリズムを用いて、lengthを求めた。　
			*/
			if (onoff == 0){
				unsigned short bb = *timebase;
				unsigned int cc = (unsigned int)bb;
				unievent[keyevent[channel][keynum]].length =( pos * 120 / cc) - unievent[keyevent[channel][keynum]].position;
				printf("%d\n", unievent[keyevent[channel][keynum]].length);
				keyevent[channel][keynum] = 0;
			}
			else if (onoff == 1){
				keyevent[channel][keynum] = eventnum;
				unievent[eventnum].position = pos * 120 / (int)*timebase; //拍子を時間に変換する作業は完了
				unievent[eventnum].channel = channel;
				unievent[eventnum].velocity = velocity;
				unievent[eventnum].key_number = keynum;
				eventnum++;
			}
			for (int ii = 1; ii <= (int)eventnum - 1; ii++){
				unievent[ii].interval = unievent[ii].position - unievent[ii - 1].position;
			}
		}

	}

	n_unievents = eventnum - 1;
	fclose(fp);

	return;

}

void UniToSMF(char *filename1, char *filename2){
	load_uni(filename1);
	write_SMF(filename2);

}

/*** main 関数 *****************************************/
//入力スタイル UniToSMF.exe [ファイル名] [ファイル名] [U or M]

int main(int argc, char *argv[]){
	if (argc <3){
		printf("不適切な入力です。\n");
		exit(-1);
		//return -1;;
	}

		printf("uniファイル%sをMIDIに変換しファイル%sに出力します。\nよろしければyを、やり直す場合はnを入力してください。\n",argv[1],argv[2]);
		char Ans;
		scanf("%s",&Ans);
		if(Ans != 'y'){
		exit(-1);
		//return -1;
  }
		UniToSMF(argv[1], argv[2]);
		return 0;
}
