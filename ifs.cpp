#include "ifs.h"
#include "coutcolor.h"


CInvertFile::~CInvertFile() {
	m_ifs.clear();
	m_imglist.clear();
	m_totalpoint.clear();
}

void CInvertFile::set_img_path(string imgpath) {
	m_img_path = imgpath;
}

void CInvertFile::set_rank_path(string rankdir) {
	m_rank_path = rankdir + "/";
	string command = "mkdir -p ";
	command = command + m_rank_path;
	system(command.c_str());
}

int CInvertFile::load_list(string listpath) {
	std::cout<< YELLOW 
    << "[notice] inverted file system: loading list ..." 
    << RESET 
    << std::endl;

	ifstream list_in(listpath.c_str(), std::ios::in);
	if (!list_in) {
		std::cerr << RED 
      << "[error] unable to load the list names:" 
			<< listpath 
      << RESET 
      << std::endl;
		return -1;
	}

	string src_name;
	while (list_in >> src_name) {
		m_imglist.push_back(src_name);
	}

	if (m_imglist.size() <= 0) {
		std::cerr << "[error] no image in the list" 
      << std::endl;
		return -1;
	}
	return 0;
}


int CInvertFile::build_ifs(int dictsize, sim_t simtype) {
	std::cout << YELLOW 
    << "[notice] inverted file system: building ifs... " 
    << RESET 
    << std::endl; 
	sim_type = simtype;
	m_is_search = 0;

	if (m_imglist.size() <= 0) {
		std::cerr << "[error] please load image list first" 
      << std::endl;
		return -2;
	}

	m_dict_size = dictsize;
	if (m_dict_size <= 0) {
		std::cerr << "[error] invalid dictionary size" 
      << std::endl;
		return -1;
	}

	int ret;
	int i = 0;

	ret = init_ifs();

	for (i = 0; i < m_imglist.size(); i++) {
		string fn;
		fn = m_imglist[i];
		ret = load_inc(fn, i);
		std::cout << "[ " 
      << i 
      << "/" 
      << m_imglist.size() 
      << "] load " 
      << m_imglist[i] 
      << " over!" 
      << std::endl;
		std::cout << "--------------------------------------------------" 
      << std::endl 
      << std::endl;
	}

	m_is_search = 1;
	std::cout << BOLDGREEN 
    << "[success] build inverted file system success" 
    << RESET 
    << std::endl;

	return 0;
}


int CInvertFile::init_ifs() {
	m_ifs.resize(m_dict_size);
	m_norms.resize(m_imglist.size(), 1);

	int imgsize = m_imglist.size();
	m_angle_diff_fwd.resize(imgsize);
	m_scale_diff_fwd.resize(imgsize);
	m_angle_diff_rvs.resize(imgsize);
	m_scale_diff_rvs.resize(imgsize);

	m_wgc_map.resize(imgsize);
	m_gvp_map.resize(imgsize);
	m_rgcm_map.resize(imgsize);

	for(int i=0; i<imgsize; i++) {
		m_angle_diff_fwd[i].resize(6);
		m_scale_diff_fwd[i].resize(6);
		m_angle_diff_rvs[i].resize(6);
		m_scale_diff_rvs[i].resize(6);
		m_wgc_map[i].resize(SCALE_BIN*ANGLE_BIN);
		m_gvp_map[i].resize(LOCATE_BIN*LOCATE_BIN);
		m_rgcm_map[i].resize(SCALE_BIN*ANGLE_BIN*LOCATE_BIN*LOCATE_BIN);
	}

	m_totalpoint.resize(imgsize);
	return 0;
}

int CInvertFile::load_inc(string file_name, index_t imgidx) {
	string filename = m_img_path + file_name + ".keyPoint";
//	std::cout << "[notice] loading image keypoints from:" << filename << std::endl;

	ifstream infea(filename.c_str(), std::ios::in);
	string temp;
	string temp1;
	float x,y;
	float scale;
	float angle;
	index_t vwidx;
	float vwvalue;
	int totalpt = 0;
	
	if(!infea) {
		std::cerr << "[load_inc]unable to load the input file" 
			<< filename
      << std::endl;
		return -1;
	}

	multimap<index_t, descp_t> query;
	while(getline(infea, temp)) {
		if(temp.size() < 5) {
			continue;
		}
		istringstream line(temp);
		line >> temp1 >> x >> y >> scale >> angle >> temp1;
		descp_t des;
		des.imgidx = imgidx;
		des.scale = scale;
		des.angle = angle;
		des.px = x;
		des.py = y;

		for (int i = 0; i < 3; i++) {
			line >> vwidx >> vwvalue;
			des.weight = vwvalue;
			des.weightrank = i+1;
			des.vwidx = vwidx;
			m_ifs[vwidx].push_back(des);
			query.insert(pair<index_t, descp_t>(vwidx, des));
		}
		totalpt++;
	}
	m_totalpoint[imgidx] = totalpt;
	std::cout << "total pt: " << totalpt << std::endl;

	CScoreDes best = self_norm(query, imgidx);
	//CScoreDes best = search_ifs(query, imgidx);
	m_norms[imgidx] = sqrt(best.score);
	std::cout << "norm: " << m_norms[imgidx] << " with filename of: " << best.file << std::endl;
	//cal norms and store them in m_norms
	return 0;

}


void CInvertFile::monitor() {
	int total = 0;
	std::cout << "--------------------------------------" << std::endl
		<< "IFS information list:" << std::endl;
	for (int i = 0; i < m_totalpoint.size(); i++) {
		total += m_totalpoint[i];
		std::cout << m_imglist[i] << " : " << m_totalpoint[i] << std::endl;
	}

	std::cout << "total sift number: " << total << std::endl
		<< "total visual words:" << m_ifs.size() << std::endl
		<< "total image number in db:" << m_imglist.size() << std::endl
	        << "--------------------------------------" << std::endl;
}

void CInvertFile::clear_wgc_hist() {
	m_scores.clear();
	for (int i = 0; i < m_imglist.size(); i++) {
		fill(m_wgc_map[i].begin(), m_wgc_map[i].end(), 0);
		fill(m_gvp_map[i].begin(), m_gvp_map[i].end(), 0);
		fill(m_rgcm_map[i].begin(), m_rgcm_map[i].end(), 0);
	}
}


CScoreDes CInvertFile::search_ifs(multimap<index_t, descp_t> &query, int i) {
	vector<index_t> query_idxvec;
	clear_wgc_hist();
	if (sim_type == BAYESPOOL) {
		rgcm_sim(query);
	} else {
		std::cout << RED 
      << "[ERROR] undefined sim_t" 
      << RESET 
      << std::endl;
	}

	sort(m_scores.begin(), m_scores.end());
	for (int j = 0; j < m_imglist.size(); j++) {
		if (m_scores[j].file == m_imglist[i]) {
			return m_scores[j];
		}
	}
	std::cout << "some error" << std::endl;
	return m_scores[0];
}


CScoreDes CInvertFile::self_norm(multimap<index_t, descp_t> &query, int i) {
	vector<index_t> query_idxvec;

	clear_wgc_hist();
	if (sim_type == BAYESPOOL) {
		self_rgcm_sim(query, i);
	} else {
		std::cout << RED 
      << "[ERROR] undefined sim_t" 
      << RESET 
      << std::endl;
	}

	sort(m_scores.begin(), m_scores.end());
	for (int j = 0; j<m_imglist.size(); j++) {
		if (m_scores[j].file == m_imglist[i]) {
			return m_scores[j];
		}
	}
	std::cout << "some error" << std::endl;
	return m_scores[0];
}


int CInvertFile::save(string query_name) {
//	std::cout << "[" << query_name << "]: rank and save" << std::endl;
	string dst_path = m_rank_path;
	dst_path += query_name;
	dst_path += ".rank";
	save_rank_list(dst_path, m_scores);
	return 0;
}

int CInvertFile::save_rank_list(string dst_path, vector<CScoreDes> &scores) {
	ofstream rnk_out(dst_path.c_str(), ios::out);
	int i;
	if (!rnk_out) {
		std::cerr << "error! unable to open the rnk_out file:" 
			<< rnk_out 
      << std::endl;
		return -1;	
	}

	for (i = 0; i < scores.size(); i++) {
		rnk_out << scores[i].file << " " << scores[i].score << std::endl;
	}

	rnk_out.close();
	return 0;
}

//similarity for bayes pooling model
int CInvertFile::rgcm_sim(multimap<index_t, descp_t> &query)
{
	std::cout << YELLOW 
    << "[notice] refining geometry constraint similarity" 
    << RESET 
    << std::endl;
	multimap<index_t, descp_t>::iterator iter;
	for (iter = query.begin(); iter != query.end(); ++iter) {
		descp_t &qdes = iter->second;
		if (qdes.vwidx != iter->first) {
			std::cerr << "[bug found] vwidx in qdes does not match with query->first" << std::endl;
			return -2;	
		}	
		vector<descp_t> &ifsdes = m_ifs[qdes.vwidx];
		vote_rgcm(qdes, ifsdes);
	}

	//store the similarity of each image
	for(int i=0; i<m_imglist.size(); i++) {
		CScoreDes tmp_score;
		vector<double> rgcmmap = m_rgcm_map[i];
		double score = merge_score_rgcm(rgcmmap);
		if (m_totalpoint[i] > 0 && m_norms[i] > 0.001) {
			tmp_score.score = score / m_norms[i];
		} else {
			tmp_score.score = 0;
		}
		tmp_score.file = m_imglist[i];
    m_scores.push_back(tmp_score);
	}
	return 0;
}


int CInvertFile::self_rgcm_sim(multimap<index_t, descp_t> &query, int i) {
	std::cout << YELLOW 
    << "[notice] Self BAYESPOOL" 
    << RESET 
    << std::endl;

	multimap<index_t, descp_t>::iterator iter;
	for (iter = query.begin(); iter != query.end(); ++iter) {
		descp_t &qdes = iter->second;
		if (qdes.vwidx != iter->first) {
			std::cerr << "[bug found] vwidx in qdes does not match with query->first" 
        << std::endl;
			return -2;	
		}	
		vector<descp_t> ifsdes;

		multimap<index_t, descp_t>::iterator it;
		int num = query.count(qdes.vwidx);
		int i;
		for (it = query.find(qdes.vwidx), i=0; i < num; ++it, i++) {
			ifsdes.push_back(it->second);
		}

		vote_rgcm(qdes, ifsdes);
		ifsdes.clear();
	}

	//store the similarity of each image
	CScoreDes tmp_score;
	vector<double> rgcmmap = m_rgcm_map[i];
	double score = merge_score_rgcm(rgcmmap);
	if (m_totalpoint[i] > 0 && m_norms[i] > 0.001) {
		tmp_score.score = score / m_norms[i];
	} else {
		tmp_score.score = 0;
	}
	tmp_score.file = m_imglist[i];
  m_scores.push_back(tmp_score);

	return 0;
}


int CInvertFile::vote_rgcm(descp_t &qdes, vector<descp_t> &ifsdesvec) {
	int i;
	index_t imgidx;
	for (i = 0; i < ifsdesvec.size(); i++) {
		descp_t &ifsdes = ifsdesvec[i];
		double sdiff = qdes.scale / ifsdes.scale;
		int adiff = qdes.angle - ifsdes.angle;
		int xdif = qdes.px - ifsdes.px;
		int ydif = qdes.py - ifsdes.py;

		imgidx = ifsdes.imgidx;

		int qa = quant_angle(adiff);
		int qs = quant_scale(sdiff);
		int qx = quant_location(xdif);
		int qy = quant_location(ydif);
		
		int iidx = qa*SCALE_BIN*(LOCATE_BIN)*(LOCATE_BIN)
      + qs*(LOCATE_BIN)*(LOCATE_BIN)
      + qx*(LOCATE_BIN)+qy;
		m_rgcm_map[imgidx][iidx] += pow(qdes.weight * ifsdes.weight, 0.8);
	}
	return 0;
}

int CInvertFile::quant_location(float x) {
	int qx;
	const float shiftmax = 20.0;
	float bins = shiftmax / (LOCATE_BIN);

	qx = (x+10.5)/bins;
	if (qx < 0) {
		qx = 0;
	} else if(qx >= LOCATE_BIN) {
		qx = LOCATE_BIN - 1;
	} else {
    //nothing
  }
	return qx;
}

//for quantization
int CInvertFile::quant_angle(int adiff) {
	adiff = (adiff + 180 / ANGLE_BIN) % 360;
	int adiffidx;
	adiffidx = adiff / (360 / ANGLE_BIN);

	if (adiffidx >= ANGLE_BIN) {
		adiffidx = ANGLE_BIN - 1;
	} else if(adiffidx < 0) {
		adiffidx = 0;
	}

	return adiffidx;
}

int CInvertFile::quant_scale(double sdiff) {
	int sdiffidx;
	if (sdiff >= 6) {
		sdiffidx = 3;
	} else if (sdiff >= 1) {
		sdiffidx = 2;
	} else if (sdiff >= 1.0/6) {
		sdiffidx = 1;
	} else {
		sdiffidx = 0;
	}

	return sdiffidx;
}

double comb(int m, int n) {
	double score = 1.0;
	for (int i = 0; i < n; i++) {
		score *= (m-i);
	}

	score = score / ranking(n);
	return score;
}

int ranking(int m) {
	int n = m;
	if (n < 1) {
		return 1;
	}
	int score = 1;
	while (n >= 1) {
		score *= n;
		n--;
	}
	return score;
}


double merge_score_rgcm(vector<double> scoremap) {
	int phrase_len = 3;
	double score = 0;

	int offset = max_element(scoremap.begin(), 
                           scoremap.end()) - scoremap.begin();

	int idx_a = offset / (LOCATE_BIN*LOCATE_BIN);	 //qa
	int shift_offset = offset % (LOCATE_BIN * LOCATE_BIN);
	int idx1 = shift_offset % LOCATE_BIN;    //qy
	int idx2 = shift_offset / LOCATE_BIN;    //qx
//  note:    int iidx = qa*SCALE_BIN*(LOCATE_BIN)*(LOCATE_BIN)+qs*(LOCATE_BIN)*(LOCATE_BIN)+qx*(LOCATE_BIN)+qy;

	float alfa1 = 2.5;
	float alfa2 = 15;
//	float alfa = 0.012;
	float r = 5;
	float alfa = 1.0/alfa1/alfa1-1.0/alfa2/alfa2;
//	std::cout << "c is: " << c << std::endl;
	for (int i=max(0, idx2-5); i<min(LOCATE_BIN, idx2+5); i++) {   //qx
		for (int j=max(0,idx1-5); j<min(LOCATE_BIN, idx1+5); j++) {  //qy
			for (int k=max(0,idx_a-5); k<min(ANGLE_BIN, idx_a+6); k++) {   //qa
				int index = k*(LOCATE_BIN*LOCATE_BIN)+i*LOCATE_BIN+j;
				int ss = (int)scoremap[index];

				float tmp;
				if (ss < phrase_len) {
					tmp = 0;
				} else {
					tmp = comb(ss, phrase_len);
				}
				int dist = (i-idx2)*(i-idx2)+(j-idx1)*(j-idx1)+(k-idx_a)*(k-idx_a);

				float weight = r*alfa1/alfa2*exp(1.0*(dist)*alfa);
				weight = 1.0 / (1 + weight);
				score += (0.95*tmp*weight + 0.05*ss);
			}
		}
	}
	return score;
}

