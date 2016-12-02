#pragma  once
#include <string>

class CScore{
public:
	CScore(float _score, std::string _file):score(_score), file(_file){}
	CScore (){}
	float score;
  std::string file;
	bool operator < (const CScore &m)const {
		return score < m.score;
	}
};

class CScoreDes{
public:
	CScoreDes (double _score, std::string _file):score(_score), file(_file){}
	CScoreDes (){}
	double score;
  std::string file;
	bool operator < (const CScoreDes &m)const {
		return score > m.score;
	}
};


