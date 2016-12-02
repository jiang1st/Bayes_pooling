#pragma  once
//constant
#define _cbNum 1000000//63108
//#define _cbNum 128//dict size:256, using PCA
//#define _cbNum 32768//dict size:256
//#define _cbNum 1048576//dict size:8192
#define _top 2000
#define _K 50
#define _Tao (6250*2)
#define _HSVdim 512

//feature matching
//#define _Unique
#include <string>
using namespace std;
class CScore{
public:
	CScore(float _score, string _file):score(_score), file(_file){}
	CScore (){}
	float score;
	string file;
	bool operator < (const CScore &m)const {
		return score < m.score;
	}
};

class CScoreDes{
public:
	CScoreDes (double _score, string _file):score(_score), file(_file){}
	CScoreDes (){}
	double score;
	string file;
	bool operator < (const CScoreDes &m)const {
		return score > m.score;
	}
};


