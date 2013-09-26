/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 * - Daan van Dongen, <Daanvdongen@gmail.com>, 2013
 */

#include <deque>
#include <iostream>
#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>
#include <dmlite/cpp/inode.h>

#include "rucio_catalog.h"
#include "rucio_connect.h"

namespace Rucio {

RucioDID::RucioDID(std::string path) : dmlite::Directory() {
  std::cerr << "[RUCIO][DID][CTOR]" << std::endl;
  std::cerr << path << std::endl;

  this->path = path;
  ptr = 0;

  stat.guid = "0";
  stat.csumtype = "ad";
  stat.csumvalue = "00000000";
  stat.stat.st_mode = S_IFDIR;
  stat.stat.st_uid = 0;
  stat.stat.st_gid = 0;
  stat.stat.st_size = 0;
  stat.stat.st_atime = 0;
  stat.stat.st_mtime = 0;
  stat.stat.st_ctime = 0;
}

RucioDID::~RucioDID() {
  std::cerr << "[RUCIO][DID][DTOR]" << std::endl;
}

RucioCatalog::RucioCatalog(dmlite::Catalog *next, std::string host, std::string port, std::string auth_token,
                           std::string ca_cert) throw (dmlite::DmException) :
  dmlite::DummyCatalog(next) {
  std::cerr << "[RUCIO][CATALOG][CTOR] " << next->getImplId() << std::endl;
  rc = new RucioConnect(host, port , auth_token, ca_cert);
  std::cerr << "https://" <<  host << ":" << port << std::endl;
}

RucioCatalog::~RucioCatalog() {
  std::cerr << "[RUCIO][CATALOG][DTOR]" << std::endl;
  delete rc;
}

std::string RucioCatalog::getImplId() const throw () {
  return "plugin_rucio/0.1";
}

void RucioCatalog::addReplica(const dmlite::Replica& replica) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][ADDREPLICA]" << std::endl;
}

void RucioCatalog::changeDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][CHANGEDIR]" << std::endl;

  std::deque<std::string> original = cwd;

  std::string tmp_path = __sanitizePath(path);
  std::cerr << tmp_path << std::endl;
  //Count and remove the '.++'
  int amount=0;
  while(tmp_path.size()>5 && tmp_path.substr(tmp_path.size()-4,4)=="/.++"){
	amount++;
	tmp_path.erase(tmp_path.size()-4,4);
  }

  for(int i=0; tmp_path.size()>4 && i<tmp_path.size()-4 ; i++){
	if(tmp_path.substr(i,4)=="/.++"){
		tmp_path.erase(i,4);
	}
  }
  tmp_path = __seperatePath(tmp_path);

  if (tmp_path == "~") { // Home is root
    if (!cwd.empty()) { // So remove everything else
      cwd.clear();
    }
  } else if (tmp_path == ".") { // Nothing to do
  } else if (tmp_path == "..") { // Go back up one
    if (!cwd.empty()) { // not necessary, if we're already at root
      cwd.erase(cwd.end());
    }
  } else { // Change to the given directory
    if (tmp_path == "/") { // Go back to root
      cwd.clear();
    } else if ((tmp_path.at(0) == '/') && (tmp_path.size() > 1)) { // Is it a full tmp_path?
      cwd = __splitPath(tmp_path);
    } else { // Or relative?
      if (tmp_path.find("/") != std::string::npos) { // Is it a hierarchy?
        if (cwd.empty()) { // Are we at root already?
          cwd.push_back(tmp_path); // If yes, just add the relative
        } else {
          cwd.push_back(tmp_path); // And the tmp_path
        }
      } else {
        std::deque<std::string> tmp_split = __splitPath(path);
        uint i = 0;
        for (uint i = 0; i < tmp_split.size(); ++i) {
          cwd.push_back(tmp_split.at(i));
        }
      }
    }
  }

  std::string tmp_scope;
  std::string tmp_did;

  std::deque<std::string> list_str;
  list_str.push_back("user");
  list_str.push_back("group");
  list_str.push_back("scope");

  char* tmp_ch = new char(1);

  if (cwd.empty()) {
    return;
  } else if (cwd.size() == 1) {					//Checking if it is a real scope
    bool realscope = TRUE;
    for( int j=0; j<list_str.size() ;j++){
	if( cwd.back() == list_str.at(j)){
		realscope = FALSE;
		break;
	    }
    }
    for (uint i = 0; i < 26 && realscope == TRUE; ++i) {	//Checking if it is a letter
	tmp_ch[0] = (char) (97+i);
	if( cwd.back() == (std::string) tmp_ch){
		realscope = FALSE;
	}
    }
    for (uint i = 0; i < 10 && realscope == TRUE; ++i) {	//Checking if it is a number
	tmp_ch[0] = (char) (48+i);
	if( cwd.back() == (std::string) tmp_ch){
		realscope = FALSE;
	}
    }

    bool found = FALSE;
    if( realscope == FALSE ){
	found = TRUE;
    } else {
	std::deque<std::string> tmp_scopes = rc->list_scopes();
	for (uint i = 0; i < tmp_scopes.size(); ++i) {
	  if (tmp_scopes.at(i) == cwd.back()) {
	    found = TRUE;
	    break;
	  }
	}
    }

    delete[] tmp_ch;

    if (!found) {
      //cwd = original;
	std::deque<std::string> tmp_sl;
	tmp_sl.push_back("///");				//Changing the name to something that doesn't exist, now Open Dir gives no data
	cwd = tmp_sl;
	std::cerr << "ERROR: Directory does not exist" <<std::endl;
      throw dmlite::DmException(DMLITE_SYSERR(1), "directory does not exist");
    }
  } else if (cwd.size() > 1) {
	std::string tmp_subscope = "";
	std::string tmp_file = "";

	if (cwd.back().at(0) == '!' ){ //grabbing the did and scope from the back
		tmp_did = cwd.back();
		tmp_did.erase(0,1);
		std::string tmp_re = cwd.back();
		cwd.pop_back();
		tmp_scope = cwd.back();
		cwd.push_back(tmp_re);
	} else {
		tmp_scope = cwd.back();
		tmp_did = "";
	}

	if( tmp_did != ""){		//looking if it is a FILE or something else
		did_t tmp_status;
		tmp_status = rc->get_did_status(tmp_scope, tmp_did);
		if(tmp_status.type == "FILE"){
			tmp_file = tmp_did;
			tmp_did = "";
			cwd.pop_back();
			cwd.pop_back();
			if( cwd.back().at(0) == '!'){
				tmp_did = cwd.back();
				tmp_did.erase(0,1);
			}
			cwd.push_back( tmp_scope) ;
			cwd.push_back( tmp_file );
		} else if( tmp_status.type != "DATASET" && tmp_status.type != "CONTAINER" ){
			std::cerr << "ERROR: Unknown did type." << std::endl;
		  }
	}

	if( tmp_subscope != ""){
		cwd.push_back( tmp_subscope);
	}

	did_t exists;
	exists = rc->get_did(tmp_scope, tmp_did);
        if (exists.scope.empty() && exists.name.empty() && exists.type.empty()) {
	     cwd = original;
	     throw dmlite::DmException(DMLITE_SYSERR(1), "directory does not exist");
	}

	if(tmp_file != ""){
		std::deque<did_t> Files;
		Files = rc->list_dids(tmp_scope, tmp_did);
		bool FileExists = FALSE;
		uint i = 0;
		for( i = 0; i<Files.size() && FileExists == FALSE ; i++){
			if( Files.at(i).name == tmp_file && Files.at(i).type == "FILE"){
				FileExists = TRUE;
				break;
			}
		}
		if (FileExists == FALSE) {
			cwd = original;
			std::cerr << "ERROR: File does not exist" << std::endl;
			throw dmlite::DmException(DMLITE_SYSERR(1), "file does not exist");
		} else {
			cwd.back() = "!" + cwd.back();
		}
	}
  }
  while(amount>0){		//put the '++' back
	amount--;
	std::string tmp_back = cwd.back();
	cwd.pop_back();
	cwd.push_back(tmp_back+"/.++");
  }
}

void RucioCatalog::closeDir(dmlite::Directory *dir) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][CLOSEDIR]" << std::endl;
  if (dir != NULL) {
    delete dir;
  }
}

void RucioCatalog::create(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][CREATE]" << std::endl;
}

dmlite::ExtendedStat RucioCatalog::extendedStat(const std::string& path, bool followSym) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][EXTENDEDSTAT]" << std::endl;
  dmlite::ExtendedStat stat_e;

  stat_e.guid = "0";
  stat_e.csumtype = "ad";
  stat_e.csumvalue = "00000000";
  //stat_e.stat.st_mode = S_IFDIR;
  stat_e.stat.st_uid = 0;
  stat_e.stat.st_gid = 0;
  stat_e.stat.st_size = 0;
  stat_e.stat.st_atime = 0;
  stat_e.stat.st_mtime = 0;
  stat_e.stat.st_ctime = 0;

  std::string tmp_path = __sanitizePath(path);
  std::cerr << tmp_path << std::endl;
  //Count and remove the '.++'
  int amount=0;
  while(tmp_path.size()>5 && tmp_path.substr(tmp_path.size()-4,4)=="/.++"){
	amount++;
	tmp_path.erase(tmp_path.size()-4,4);
  }
  for(int i=0; tmp_path.size()>4 && i<tmp_path.size()-4 ; i++){
	if(tmp_path.substr(i,4)=="/.++"){
		tmp_path.erase(i,4);
	}
  }
  tmp_path = __seperatePath(tmp_path);

  if (tmp_path == "/") {
    stat_e.name = "/";
    while(amount>0){		//put the '++' back
	amount--;
std::cerr << "we do do this" << std::endl;
	stat_e.name.append("/.++");
    }
    stat_e.stat.st_mode = S_IFDIR;
    return stat_e;
  }

  std::deque<std::string> tokens;
  std::string::size_type tokenOff = 0, sepOff = tokenOff, tmp_sit = 0;
  while (sepOff < tmp_path.npos) {
        tmp_sit = tmp_path.find('/', sepOff);
        sepOff = tmp_sit;
        std::string::size_type tokenLen = (sepOff == std::string::npos) ? sepOff : sepOff++ - tokenOff;
        std::string token = tmp_path.substr(tokenOff, tokenLen);
        if (!token.empty()) {
          tokens.push_back(token);
        }
        tokenOff = sepOff;
  }

  std::string tmp_file = "";
  std::string tmp_scope = "";
  if(path.at(0) != '/'){	//pushing the cwd in the front of the path
      for( int i=0 ; i< (int)(cwd.size())-1 ; i++){
		tokens.push_front(cwd.at(cwd.size()-2-i));
      }
  }

  if(tokens.back().at(0) == '!'  ){
		tmp_file = tokens.back();
		tmp_file.erase(0,1);
		tokens.pop_back();
		tmp_scope = tokens.back();
		did_t tmp_status;
		tmp_status = rc->get_did_status(tmp_scope, tmp_file);
		if(tmp_status.type == "FILE"){
			stat_e.stat.st_mode = S_IFREG;
			stat_e.name = tmp_file;
			while(amount>0){		//put the '++' back
				amount--;
				stat_e.name.append("/.++");
			}
			std::deque<replica_t> tmp_deq_r;
			tmp_deq_r = rc->list_replicas(tmp_scope, tmp_file);
			if( tmp_deq_r.size() > 0){
				stat_e.stat.st_size = tmp_deq_r.at(0).size;
			}
		}
  }

  if(stat_e.stat.st_mode != S_IFREG){
	stat_e.stat.st_mode = S_IFDIR;
  }
  return stat_e;
}

std::string RucioCatalog::getComment(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETCOMMENT]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
  return std::string();
}

dmlite::Replica RucioCatalog::getReplica(const std::string *rfn) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETREPLICA]" << std::endl;
  dmlite::Replica dummy;
  return dummy;
}

std::vector<dmlite::Replica> RucioCatalog::getReplicas(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETREPLICAS]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
  std::cerr << tmp_path << std::endl;
  for(int i=0; tmp_path.size()>4 && i<tmp_path.size()-4 ; i++){
	if(tmp_path.substr(i,4)=="/.++"){
		tmp_path.erase(i,4);
	}
  }
  tmp_path = __seperatePath(tmp_path);

  std::deque<std::string> tokens;
  std::string::size_type tokenOff = 0, sepOff = tokenOff, tmp_sit = 0;
  while (sepOff < tmp_path.npos) {
        tmp_sit = tmp_path.find('/', sepOff);
        sepOff = tmp_sit;
        std::string::size_type tokenLen = (sepOff == std::string::npos) ? sepOff : sepOff++ - tokenOff;
        std::string token = tmp_path.substr(tokenOff, tokenLen);
        if (!token.empty()) {
          tokens.push_back(token);
        }
        tokenOff = sepOff;
  }

  std::string tmp_scope = "";
  std::string tmp_file = "";

  if(path.at(0) != '/'){	//pushing the cwd in the front of the path
      for( int i=0 ; i< (int)(cwd.size())-1 ; i++){
		tokens.push_front(cwd.at(cwd.size()-2-i));
      }
  }

  if(tokens.back().at(0) == '!'  ){
		tmp_file = tokens.back();
		tmp_file.erase(0,1);
		tokens.pop_back();
		tmp_scope = tokens.back();
  }

  std::vector<dmlite::Replica> tmp_repl;
  std::deque<replica_t> tmp_deq_r;
  tmp_deq_r = rc->list_replicas(tmp_scope, tmp_file);
  for( int i=0 ; i<tmp_deq_r.size() ; i++){
	dmlite::Replica replica;
	replica.server	= "ThisIsAServername";			//metalink entry
	replica.rfn	= "--" + tmp_deq_r.at(i).RSE + "--" ;	//metalink entry
	tmp_repl.push_back(replica);
  }
  return tmp_repl;
}

std::string RucioCatalog::getWorkingDir() throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETWORKINGDIR]" << std::endl;
  std::string cwd_s = "/";
  for (int i = 0; i < cwd.size(); ++i) {
    cwd_s += cwd.at(i) + "/";
  }

  if (cwd_s != "/") {
    cwd_s = cwd_s.substr(0, cwd_s.size() - 1);
  }
  return cwd_s;
}

void RucioCatalog::makeDir(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][MAKEDIR]" << std::endl;
}

dmlite::Directory *RucioCatalog::openDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][OPENDIR]" << std::endl;

  std::string tmp_path = __sanitizePath(path);
  std::cerr << tmp_path << std::endl;

  uint did_r_per = 50;
  //Count and remove the '.++'
  int amount=0;
  while(tmp_path.size()>3 && tmp_path.substr(tmp_path.size()-4,4)=="/.++"){
	amount++;
	tmp_path.erase(tmp_path.size()-4,4);
  }

  uint did_r_start=did_r_per*amount;
  /*for( uint i = 0; i<tmp_path.size()-1 ; i++){
	if(tmp_path.at(i)== '-' && tmp_path.at(i+1)!='-'){
		std::string tmp_strint = tmp_path.substr(i+1, tmp_path.npos-i-1);
		tmp_path.erase(i-1, tmp_path.npos-i+1);
		did_r_start += std::strtoul(tmp_strint.c_str(),NULL,10);
		break;
	}
  }*/
  for(int i=0; tmp_path.size()>4 && i<tmp_path.size()-4 ; i++){
	if(tmp_path.substr(i,4)=="/.++"){
		tmp_path.erase(i,4);
	}
  }
  tmp_path = __seperatePath(tmp_path);

  RucioDID *did_r = new RucioDID(tmp_path);

  std::cerr << tmp_path << std::endl;

  std::deque<std::string> tokens;		//tokens = your path
  bool realscope= TRUE;

  std::deque<std::string> list_str;
  list_str.push_back("user");
  list_str.push_back("group");
  list_str.push_back("scope");

  did_r->scopes.push_back(".");
  did_r->scopes.push_back("..");
  did_r->dids.push_back(std::string());
  did_r->dids.push_back(std::string());
  did_r->types.push_back(std::string());
  did_r->types.push_back(std::string());
  did_r->RSE.push_back(std::string());
  did_r->RSE.push_back(std::string());

    /**
   * The root directory is special, because it's only a list of scopes
   */

  if (tmp_path == "/") {
    for( int j=0; j<list_str.size() ; j++){
	did_r->scopes.push_back( list_str.at(j) );
	did_r->dids.push_back(std::string());
	did_r->types.push_back(std::string());
	did_r->RSE.push_back(std::string());
    }

    char* tmp_ch2 = new char(1);
    for (uint i = 0; i < 26; ++i) {	//pushing back every letter
	tmp_ch2[0] = (char) (97+i);
	did_r->scopes.push_back( (std::string) tmp_ch2 );
	did_r->dids.push_back(std::string());
	did_r->types.push_back(std::string());
	did_r->RSE.push_back(std::string());
    }
    for (uint i = 0; i < 10; ++i) {	//pushing back every number
	tmp_ch2[0] = (char) (48+i);
	did_r->scopes.push_back( (std::string) tmp_ch2 );
	did_r->dids.push_back(std::string());
	did_r->types.push_back(std::string());
	did_r->RSE.push_back(std::string());
    }
    delete[] tmp_ch2;

  } else {
      std::string::size_type tokenOff = 0, sepOff = tokenOff, tmp_sit = 0;
      while (sepOff < tmp_path.npos) {
        tmp_sit = tmp_path.find('/', sepOff);
        sepOff = tmp_sit;
        std::string::size_type tokenLen = (sepOff == std::string::npos) ? sepOff : sepOff++ - tokenOff;
        std::string token = tmp_path.substr(tokenOff, tokenLen);
        if (!token.empty()) {
          tokens.push_back(token);
        }
        tokenOff = sepOff;
      }

      if(path.at(0) != '/'){	//pushing the cwd in the front of the path
	      for( int i=0 ; i< (int)(cwd.size())-1 ; i++){
			tokens.push_front(cwd.at(cwd.size()-2-i));
	      }
      }

	/*
	Checking if scopes need to be listed, or if it is a real existing scope
	*/
    std::string tmp_str = "";
    realscope = TRUE;
    if (tokens.size() == 1) {
	for( int j=0; j<list_str.size();j++){
	        if( tokens.back() == list_str.at(j)){
			tmp_str = list_str.at(j);
			realscope = FALSE;
			break;
		}
	}
	for (uint i = 0; i < 26 && realscope == TRUE; ++i) {
		tmp_str = (char) (97+i);
		if( tokens.back() == tmp_str){
			realscope =FALSE;
			tmp_str = tmp_str;
		}
	}
	for (uint i = 0; i < 10 && realscope == TRUE; ++i) {
		tmp_str = (char) (48+i);
		if( tokens.back() == tmp_str){
			realscope =FALSE;
			tmp_str = tmp_str;
		}
	}
    }

    uint did_r_max;
    if(did_r_start==0 && amount==0){
	//did_r_max= did_r_per;
	did_r_max=0-1;
    } else {
	did_r_max = did_r_start;
	did_r_start = did_r_start-did_r_per;
    }
    uint did_r_count=0;		//Does not count subscopes!

    if(realscope == FALSE ) {
    /**
     * The listing of scopes based on there starting chars
     */
	std::deque<std::string> tmp_scopes = rc->list_scopes();

	bool done = FALSE;
	for(int j=0; j<list_str.size() && done==FALSE ;j++){
		if( tmp_str == list_str.at(j)){
			done = TRUE;
			for (uint i = 0; i < tmp_scopes.size(); ++i) {
				if(did_r_count>did_r_max) break;
				if( UpToLow(tmp_scopes.at(i).substr(0,list_str.at(j).size())) == list_str.at(j) ){	//checking if it starts with "user"
						did_r_count++;
						if(did_r_count> did_r_start){
							did_r->scopes.push_back("/" + tmp_scopes.at(i));
							did_r->dids.push_back(std::string());
							did_r->types.push_back("SCOPE");
							did_r->RSE.push_back(std::string());
						}
				}
			}
		}
	}
	for(int j=0; j<list_str.size() && done==FALSE ;j++){
		if(list_str.at(j).size()<1) continue;
		if( tmp_str == list_str.at(j).substr(0,1)){
			done=TRUE;
			for (uint i = 0; i < tmp_scopes.size(); ++i) {
				if(did_r_count>did_r_max) break;
				if(UpToLow(tmp_scopes.at(i).substr(0,1)) == list_str.at(j).substr(0,1) && !(UpToLow(tmp_scopes.at(i).substr(0,list_str.at(j).size())) == list_str.at(j)) ){	//checking if it starts with "u", but not "user"
					did_r_count++;
					if(did_r_count> did_r_start){
						did_r->scopes.push_back("/" + tmp_scopes.at(i));
						did_r->dids.push_back(std::string());
						did_r->types.push_back("SCOPE");
						did_r->RSE.push_back(std::string());
					}
				}
			}
		}
	}
	if(done == FALSE){
		for (uint i = 0; i < tmp_scopes.size(); ++i) {
			if(did_r_count>did_r_max) break;
			if( UpToLow(tmp_scopes.at(i).substr(0,1)) == tmp_str){
				did_r_count++;
				if(did_r_count> did_r_start){
					did_r->scopes.push_front("/" + tmp_scopes.at(i));
					did_r->dids.push_front(std::string());
					did_r->types.push_front("SCOPE");
					did_r->RSE.push_front(std::string());
				}
			}
		}
	}

	if(did_r_max <= did_r->scopes.size() || did_r_max-did_r_start > did_r_per){
		did_r->scopes.push_front(".++");
		did_r->dids.push_front(std::string());
		did_r->types.push_front("SCOPE");
		did_r->RSE.push_front(std::string());
	}
    } else {			//If realscope==TRUE
    /**
     * Everything else is a combination of scope, DID, or both. So split it up and let's see what we've got.
     */



      /**
       * Just look at the last two entries.
       */
      std::string tmp_subscope = "";
      std::string tmp_scope = "";
      std::string tmp_did = "";

      if(tokens.back().at(0) != '!' ){
	tmp_subscope = tokens.back();		//taking the subscope off
	if( tokens.size() > 1 ){
		tokens.pop_back();
		if( tokens.back().at(0) != '!' ){
			tmp_scope = tmp_subscope;
			tmp_subscope = "";
		}
	} else {
		tmp_scope = tmp_subscope;
		tmp_subscope = "";
	}
      }

      if(tmp_did == "" && tokens.back().at(0) == '!'){
		tmp_did = tokens.back();
		tmp_did.erase(0,1);
		if( tmp_scope == "" ){
			std::string tmp_re = tokens.back();
			tokens.pop_back();
			tmp_scope = tokens.back();
			tokens.push_back(tmp_re);
		}
      }

      if(tmp_scope == ""){
		while(tokens.back().at(0) == '!'){
			tokens.pop_back();
		}
		tmp_scope = tokens.back();
      }

      if( tmp_did != ""){
		did_t tmp_status;
		tmp_status = rc->get_did_status(tmp_scope, tmp_did);
		if(tmp_status.type == "FILE"){
			std::cerr << "ERROR: Driectory to open is of FILE type." << std::endl;
			return did_r;
		} else if( tmp_status.type != "DATASET" && tmp_status.type != "CONTAINER" ){
			std::cerr << "ERROR: Unknown did type." << std::endl;
		  }
      }

      if(tmp_did != ""){
		tokens.push_back("!" + tmp_did);
      }

      if ((tmp_scope == ".") && (tmp_did == ".")) {
        return did_r;
      }

      std::deque<did_t> tmp_r;
      tmp_r = rc->list_dids(tmp_scope, tmp_did);

      if(did_r_max <= tmp_r.size() || did_r_max-did_r_start > did_r_per){
		did_r->scopes.push_back(".++");
		did_r->dids.push_back(std::string());
		did_r->types.push_back("SCOPE");
		did_r->RSE.push_back(std::string());
      }
      std::string tmp_scope_compare;
      if( tmp_subscope == "" ){
		tmp_scope_compare = tmp_scope;
      } else {
		tmp_scope_compare = tmp_subscope;
      }

      for (uint i = 0; i < tmp_r.size(); ++i) { //Checking if current scope is the same as the scope of the listed dids
	if(did_r_count>did_r_max) break;
	std::string tmp_str = tmp_r.at(i).scope;
	if( tmp_str.substr(0,1) == "/" ){
		tmp_str.erase(0,1);
        }

	if( tmp_scope_compare == tmp_str ){
		did_r_count++;
		if(did_r_count> did_r_start){
			did_r->scopes.push_back(tmp_r.at(i).scope );
			did_r->dids.push_back(tmp_r.at(i).name);
			did_r->types.push_back(tmp_r.at(i).type);
			did_r->RSE.push_back(tmp_r.at(i).RSE);
		}
	} else if (tmp_subscope == ""){						//Don't do this if you're allready in a subcategory
		bool SubScopeExists = FALSE;					//Check if it is not allready there
		for( uint jj =0 ; jj < did_r->scopes.size() && SubScopeExists == FALSE; jj++){
			if( did_r->scopes.at(jj) == ( tmp_str ) ){
				SubScopeExists = TRUE;
			}
		}
		if( SubScopeExists == FALSE ){
			did_r->scopes.push_back( tmp_str );
			did_r->dids.push_back("");
			did_r->types.push_back("");
			did_r->RSE.push_back("");
		}
	}
      }
    }
  }

  if(tokens.size()>0){
	if( realscope == FALSE){
		tokens.pop_front();
	}
	for(uint i = 0; i< tokens.size() ; i++){
		if(tokens.at(i).at(0) == '!'){
			tokens.at(i).erase(0,1);
		}
	}
	cwd=tokens;
  }
  return did_r;
}

struct dirent *RucioCatalog::readDir(dmlite::Directory *dir) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][READDIR]" << std::endl;

  return NULL;
}

dmlite::ExtendedStat *RucioCatalog::readDirx(dmlite::Directory *dir) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][READDIRX]" << std::endl;

  RucioDID *did_r = dynamic_cast<RucioDID *> (dir);

  if ((did_r->ptr == did_r->scopes.size()) || (did_r->scopes.empty())) {
    return NULL;
  }

  if( did_r->dids.at(did_r->ptr) != ""){
	std::cerr << did_r->dids.at(did_r->ptr)<< std::endl;
  } else {
	std::cerr << did_r->scopes.at(did_r->ptr)<< std::endl;
  }

  if (did_r->path == "/") {
    did_r->stat.name = did_r->scopes.at(did_r->ptr);
  } else {
    if (did_r->scopes.at(did_r->ptr).empty()) {
      did_r->stat.name = did_r->dids.at(did_r->ptr);
    } else {
      if (did_r->dids.at(did_r->ptr).empty()) {
        did_r->stat.name = did_r->scopes.at(did_r->ptr);
      } else {
	if (did_r->types.at(did_r->ptr).empty() ){
		std::cerr << "ERROR: " << did_r->dids.at(did_r->ptr) << " DID without type" <<std::endl;
	} else {
	  if (did_r->types.at(did_r->ptr) == "CONTAINER" || did_r->types.at(did_r->ptr) == "DATASET" ){
		did_r->stat.name = did_r->scopes.at(did_r->ptr) + "--" + did_r->dids.at(did_r->ptr);
	} else {
	    if (did_r->types.at(did_r->ptr) == "FILE" ){
		did_r->stat.name =  did_r->scopes.at(did_r->ptr) + "--" + did_r->dids.at(did_r->ptr);
		did_r->stat.stat.st_mode = S_IFREG;
	    } else {
		std::cerr << "ERROR: Unknown DID-type" << std::endl;
		did_r->stat.name = did_r->scopes.at(did_r->ptr);
	    }
	  }
        }
      }
    }
  }

  ++did_r->ptr;
  return &did_r->stat;
}

std::string RucioCatalog::readLink(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][READLINK]" << std::endl;
  std::string tmp_path = __sanitizePath(path);

  return std::string();
}

void RucioCatalog::removeDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][REMOVEDIR]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::rename(const std::string& oldPath, const std::string& newPath) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][RENAME]" << std::endl;
  std::string tmp_oldPath = __sanitizePath(oldPath);
  std::string tmp_newPath = __sanitizePath(newPath);
}

void RucioCatalog::setAcl(const std::string& path, const dmlite::Acl& acl) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETACL]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setChecksum(const std::string& path, const std::string& csumtype, const std::string &
                               csumvalue) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETCHECKSUM]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setComment(const std::string& path, const std::string& comment) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETCOMMENT]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setGuid(const std::string& path, const std::string& guid) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETGUID]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setMode(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETMODE]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setOwner(const std::string& path, uid_t newUid, gid_t newGid,
                            bool followSymlink) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETOWNER]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setSecurityContext(const dmlite::SecurityContext *ctx) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSECURITYCONTEXT]" << std::endl;
}

void RucioCatalog::setSize(const std::string& path, size_t newSize) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSIZE]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setStackInstance(dmlite::StackInstance *si) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSTACKINSTANCE]" << std::endl;
}

void RucioCatalog::symlink(const std::string& path, const std::string symlink) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SYMLINK]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

mode_t RucioCatalog::umask(mode_t mask) throw () {
  std::cerr << "[RUCIO][CATALOG][UMASK]" << std::endl;
  return 0;
}

void RucioCatalog::unlink(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UNLINK]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::updateExtendedAttributes(const std::string& path, const dmlite::Extensible &
                                            attr) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UPDATEEXTENDEDATTRIBUTES]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::updateReplica(const dmlite::Replica& replica) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UPDATEREPLICA]" << std::endl;
}

void RucioCatalog::utime(const std::string& path, const struct utimbuf *buf) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UTIME]" << std::endl;

  std::string tmp_path = __sanitizePath(path);
}

std::string RucioCatalog::__sanitizePath(std::string path) {

  std::string tmp_path;

  if (path == ".") {
    tmp_path = "/";
    for (uint i = 0; i < cwd.size(); ++i) {
      tmp_path = cwd.at(i) + "/";
    }
    if (tmp_path != "/") {
      tmp_path = tmp_path.substr(0, tmp_path.size() - 1);
    }
    return tmp_path;
  }

  tmp_path = path;
  size_t f;

  while ((f = tmp_path.find("//")) != std::string::npos) { // Remove double slashes
    tmp_path.replace(f, 2, std::string("/"));
  }

  if ((tmp_path.size() > 1) && (tmp_path.at(tmp_path.size() - 1) == '/')) { // Remove slash at the end
    tmp_path.erase(tmp_path.size() - 1);
  }

  return tmp_path;
}

std::string RucioCatalog::__seperatePath(std::string path){
	std::string tmp_path = path;
	if( tmp_path.size()>2){
		for( uint i = 0; i<tmp_path.size()-2 ; i++){
			if(tmp_path.substr(i,2)== "--"){
				tmp_path.at(i)='/';
				tmp_path.at(i+1)='!';
			}
		}
	}
	return tmp_path;
}

std::deque<std::string> RucioCatalog::__splitPath(std::string path) {
  std::deque<std::string> tmp_split;

  size_t prev = 0;
  size_t next = 0;

  while ((next = path.find_first_of("/", prev)) != std::string::npos) {
    if ((next - prev) != 0) {
      tmp_split.push_back(path.substr(prev, next - prev));
    }
    prev = next + 1;
  }

  if (prev < path.size()) {
    tmp_split.push_back(path.substr(prev));
  }

  return tmp_split;
}
}

std::string UpToLow(std::string str) {
    for (int i=0;i<strlen(str.c_str());i++)
        if (str[i] >= 0x41 && str[i] <= 0x5A)
            str[i] = str[i] + 0x20;
    return str;
}

std::string to_string(int number){
    std::string number_string = "";
    char ones_char;
    int ones = 0;
    while(true){
        ones = number % 10;
        switch(ones){
            case 0: ones_char = '0'; break;
            case 1: ones_char = '1'; break;
            case 2: ones_char = '2'; break;
            case 3: ones_char = '3'; break;
            case 4: ones_char = '4'; break;
            case 5: ones_char = '5'; break;
            case 6: ones_char = '6'; break;
            case 7: ones_char = '7'; break;
            case 8: ones_char = '8'; break;
            case 9: ones_char = '9'; break;
            default : ;
        }
        number -= ones;
        number_string = ones_char + number_string;
        if(number == 0){
            break;
        }
        number = number/10;
    }
    return number_string;
}
