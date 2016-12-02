#ifndef _INVERTED_FILE_SYSTEM_H_
#define _INVERTED_FILE_SYSTEM_H_


#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "utils.h"

using namespace std;


typedef int index_t;

#define ANGLE_BIN 2
#define SCALE_BIN 4
#define LOCATE_BIN 10

enum sim_t {
	L2 = 0,
	L1,
	WGC,
	GVP,
	BAYESPOOL
};

struct descp_t {
	index_t imgidx;
	index_t vwidx;       //visual word index in dictionary
	float scale;     //scale for SIFT
	float angle;     //angle for SIFT
	float weight;    //weight of soft projection
	int weightrank;  //from 1 to 3
	float px;        //x cordinate for SIFT
	float py;        //y cordinate for SIFT
};

typedef multimap<index_t, descp_t>::iterator idx_map_iter;

class CInvertFile {
public:

	CInvertFile(){};
	~CInvertFile();

	int load_list(string listpath);
	int build_ifs(int dictsize, sim_t simtype);
	int load_inc(string file_name, index_t imgidx);
	void monitor();
	void set_img_path(string imgpath);
	void set_rank_path(string rankdir);
	int save(string queryName);

//	int get_dbimg_size(); 
	CScoreDes search_ifs(multimap<index_t, descp_t> &query, int i=0);
	int m_is_search;

private:
	int init_ifs();

	void clear_wgc_hist();

	//from query -> search ifs
	int forward_sim(multimap<index_t, descp_t> &query, vector<index_t> &query_idxvec); 

      	//from ifs   -> search query
	int reverse_sim(multimap<index_t, descp_t> &query, vector<index_t> &query_idxvec);  

	int combine_sim();
	
	int vote_consistency(descp_t &qdes, vector<descp_t> &ifsdesvec);

	int vote_consistency(pair<idx_map_iter, idx_map_iter> &qpair, descp_t &ifsdes);

//	int vote_diff_hist(float sdiff, float adiff, index_t imgidx);

	int vote_diff_hist(float sdiff, float adiff, vector<vector<int> > &scalediff, vector<vector<int> > &anglediff, index_t imgidx);

	int vote_merge_fwd(vector<vector<int> > &scalediff, vector<vector<int> > &anglediff);


	int save_rank_list(string dstPath, vector<CScoreDes> &scores);
	
	//for weak geometry consistency
	int wgc_sim(multimap<index_t, descp_t> &query);
	int vote_wgc(descp_t &qdes, vector<descp_t> &ifsdesvec);

	//for refining geometry constraint model
	int rgcm_sim(multimap<index_t, descp_t> &query);
	int vote_rgcm(descp_t &qdes, vector<descp_t> &ifsdesvec);

	//for l2 similarity
	int l2_sim(multimap<index_t, descp_t> &query);
	int vote_l2(descp_t &qdes, vector<descp_t> &ifsdesvec);

	//for l2 similarity
	int gvp_sim(multimap<index_t, descp_t> &query);
	int vote_gvp(descp_t &qdes, vector<descp_t> &ifsdesvec);
	
	
	CScoreDes self_norm(multimap<index_t, descp_t> &query, int i);
	int self_l2_sim(multimap<index_t, descp_t> &query, int i);
	int self_wgc_sim(multimap<index_t, descp_t> &query, int i);
	int self_gvp_sim(multimap<index_t, descp_t> &query, int i);
	int self_rgcm_sim(multimap<index_t, descp_t> &query, int i);

	//quantize angle and scale diff	
	int quant_angle(int adiff);
	int quant_scale(double sdiff);
	int quant_location(float x);


	int m_dict_size;
	string m_img_path;                  	   //path that stores keypoints
	string m_rank_path;                        //path that stores ranked list
	vector<string> m_imglist;           	   //image name list
	vector<int> m_totalpoint;
	vector<vector<descp_t> > m_ifs;      	   //inverted file that keeps SIFT
	

	vector<vector<int> > m_angle_diff_fwd;    //WGC hist that keeps angle diff
	vector<vector<int> > m_scale_diff_fwd;    //WGC hist that keeps scale diff
	vector<vector<int> > m_angle_diff_rvs;    //WGC hist that keeps angle diff
	vector<vector<int> > m_scale_diff_rvs;    //WGC hist that keeps scale diff

	vector<vector<double> > m_wgc_map;
	vector<vector<double> > m_gvp_map;
	vector<vector<double> > m_rgcm_map;

	vector<CScoreDes> m_scores;

	vector<double> m_norms;

	sim_t sim_type;
};

#endif


inline double comb(int m, int n);
inline int ranking(int n);
inline double merge_score_comb(vector<double> scoremap);
inline double merge_score_vpl(vector<double> scoremap);
inline double merge_score_rgcm(vector<double> scoremap);
inline double merge_score_combmax(vector<double> scoremap);
inline double merge_score_max(vector<double> scoremap);
inline double merge_score_sum(vector<double> scoremap);
