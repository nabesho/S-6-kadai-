
#include <stdio.h>
#include <stdlib.h>
#include "uni.h"
#include "stdafx.h"


//�g���Ă���v�Z�@�����g���G���f�B�A����p���Ă��邽�߁A�G���f�B�A���̕ϊ����s���֐��������B
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


//write_uni�͉ۑ�2�ō쐬�������̂�p�����B
void write_uni(char *filename){

	FILE *fp;

	int i;

	if ((fp = fopen(filename, "w")) == NULL){
		exit(-1);
	}
	else{

		for (i = 1; i <= n_unievents; i++){
			if (unievent[i].key_number != -1 || unievent[i].measure != 0){ //�C�x���g�̗L���̔��ʂ̂���keynumber��measure���t���O�Ƃ��ėp����
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


/*
SMF�`������unievents�P�ʂɗ��Ƃ��悤�Ȋ֐����쐬�����B
�ώG�ɂȂ��Ă͂��܂������̂́A�|�C���^��fread�֐���p���ăo�C�i���t�@�C������l�����o����Ƃ��J��Ԃ�
uni�t�@�C���ɗ��Ƃ����߂�悤�Ȍ`�ɂ��邱�Ƃ��ł����B
*/
void load_SMF(char *filename){
	int z = 0;
	int *buf[100]; //����Ȃ��l���̂Ă邽�߂̕ϐ�
	buf[0] = &z;

	for (int i = 1; i < 10; i++){
		*(buf + 4*i) = 0;
	}

	FILE *fp;
	unsigned short aa = 0; //�G���f�B�A���ϊ��̊֌W�Ŋ�{�I�ɕϐ���unsigned��p�����B
	unsigned short *numoftrack = &aa; //�g���b�N�����i�[����ϐ�
	unsigned short ab = 0;
	unsigned short *timebase = &ab;//�^�C���x�[�X���i�[����ϐ�

	if ((fp = fopen(filename, "rb")) == NULL){
		printf("�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B\n");
		exit(-1);
	}
	fread(buf, 10, 1, fp);
	fread(numoftrack, 2, 1, fp);
	*numoftrack = sconv(*numoftrack); //�K�X�G���f�B�A���ϊ����s���A�ق����l�𒊏o����

	fread(timebase, 2, 1, fp);
	*timebase = sconv(*timebase);
	unsigned int datalength, datanum, eventnum, keyevent[16][256], velocity, pos, channel, onoff, keynum;
	/*
	datalength:�f�[�^��
	datanum:��������ł���ꏊ�����o�C�g�ڂ���\��
	velocity,pos,channel,keynum�̓m�[�g�̃X�e�[�^�X
	onoff��0�Ńv�b�V���A1�Ńv�[���A2�ł���ȊO�̓����\���B
	keyevent�͌�q����B
	*/
	eventnum = 1;
	for (int i = 0; i < 16; i++){
		for (int j = 0; j < 256; j++){
			keyevent[i][j] = 0;
		}
	}
	pos = 0;

	for (int i = 1; i <= *numoftrack; i++){
		unsigned long deltatime; //�f���^�^�C����\��
		unsigned short tempdelta; //�ϒ����������邽�߂ɗp����B
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
		�ϒ��̒l����{���̒l�����o�����߁A1�o�C�g���ǂݍ����tempdelta�Ɋi�[���A
		deltatime��128�{����tempdelta��127�ȉ��ɂȂ�܂ő������Ƃ��J��Ԃ��Ƃ������@���Ƃ����B
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
			pos += deltatime; //pos�̓�deltatime�Aposition�Ɠ��������Ă���B
			unsigned char ae = 0;
			unsigned char *cnl = &ae;
			fread(cnl, 1, 1, fp);
			datanum += 1;
			channel = (unsigned int)*cnl;
			//channel�͒l�̑傫���ɂ���ĕ]�����s���A�ق����l��onoff�̎��ʂɕ������B
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
			���Ղ��������藣�����肷��A�Ƃ�����Ƃ�length�Ƃ��Ď�荞�ށB��������͐�Η�����Ƃ�����͂��B
			�Ƃ������ƂŁA16�`�����l��256�L�[���̔z���p�ӂ���B
			����`�����l���̃L�[�������ꂽ��A���̃`�����l���ƃL�[�ɑΉ�����ӏ��ɁA�����ꂽ�C�x���g�ԍ����i�[����B
			���ɂ��̃`�����l���̃L�[�������ꂽ��A�i�[�����l����length���v�Z���A�i�[���Ă����ӏ��̒l��0�ɖ߂��B
			�ȏ�̂悤�ȃA���S���Y����p���āAlength�����߂��B�@
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
				unievent[eventnum].position = pos * 120 / (int)*timebase; //���q�����Ԃɕϊ������Ƃ͊���
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

/*
unievents�̌`����A���S���Y����fwrite�A�|�C���^��K�؂ɗ��p����SMF�t�@�C�����쐬�����B
*/

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
	SMF�t�@�C�����ł́Auni�t�@�C����unievent�ɂ͂Ȃ��u���Ղ𗣂��v�Ƃ������삪����B
	�����ŁA���ׂĂ�unievent�ɑ΂��Ă�����C�x���g������channel��-1�{���邱�ƂŁu����������v�ƒ�`�����B
	�܂����������position�͉��������position+length�ɂȂ�̂͌����܂ł��Ȃ��B
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
		position�Ő��`�T�����ēK�؂ȏ��ԂɂȂ�悤�ɃC�x���g���\�[�g�����B
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
	SMF�ɂ͏����Ȃǂ̓T�|�[�g���Ȃ��\��ł������̂ŁA�m�[�g�C�x���g���m������interval���v�Z�����B�̂��ɗ��p����B
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
	//�ϒ��Ȓl�̊i�[�́A�K�؂ɕϐ���p���A�ŏ�ʂ̃r�b�g��������܂��s���ē��삳�����B
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

void UniToSMF(char *filename1, char *filename2){
	load_uni(filename1);
	write_SMF(filename2);

}

void SMFToUni(char *filename1, char *filename2){
	load_SMF(filename1);
	write_uni(filename2);
}




/*** main �֐� *****************************************/
//���̓X�^�C�� UniToSMF.exe [�t�@�C����] [�t�@�C����] [U or M]

int main(int argc, char *argv[]){
	if (argc <4){
		printf("�s�K�؂ȓ��͂ł��B\n");
		exit(-1);
		//return -1;;
	}

	if (*argv[3] == 'U'){
		printf("uni�t�@�C��%s��MIDI�ɕϊ����t�@�C��%s�ɏo�͂��܂��B\n��낵�����y���A��蒼���ꍇ��n����͂��Ă��������B\n",argv[1],argv[2]);
		char Ans;
		scanf("%s",&Ans);
		if(Ans != 'y'){
		exit(-1);
		//return -1;
		}
		UniToSMF(argv[1], argv[2]);
		return 0;
	}
	else if (*argv[3] == 'M'){
		printf("MIDI�t�@�C��%s��uni�ɕϊ����t�@�C��%s�ɏo�͂��܂��B\n��낵�����y���A��蒼���ꍇ��n����͂��Ă��������B\n",argv[1],argv[2]);
		char Ans;
		scanf("%s",&Ans);
		if(Ans != 'y'){
		exit(-1);
		}
		SMFToUni(argv[1], argv[2]);
		return 0;
	}
	else{
		printf("���͂��s�K�؂ł��B\n");
		exit(-1);
		//return -1;
	}




}
