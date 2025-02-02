#include "dsl/type_description.hpp"
#include "dsl/statement.hpp"

DescriptionDB::DescriptionDB() {
  Description *descriptor;
  // adding types with no template parameters
  descriptor = new TypeData64B(); add("Data64B", descriptor); descriptor->emit_header();
}

DescriptionDB::~DescriptionDB() {
  for(auto t:types) delete t.second;
}

bool DescriptionDB::create(const std::string &type_name, const std::string &base_name, std::list<std::string> &params) {
  Description *descriptor = nullptr;
  if(base_name == "MetadataMSI")           descriptor = new TypeMetadataMSI(type_name);
  if(base_name == "Data64B")               descriptor = new TypeData64B(type_name);
  if(base_name == "CacheArrayNorm")        descriptor = new TypeCacheArrayNorm(type_name);
  if(base_name == "CacheSkewed")           descriptor = new TypeCacheSkewed(type_name);
  if(base_name == "CacheNorm")             descriptor = new TypeCacheNorm(type_name);
  if(base_name == "OuterPortMSIUncached")  descriptor = new TypeOuterPortMSIUncached(type_name);
  if(base_name == "OuterPortMSI")          descriptor = new TypeOuterPortMSI(type_name);
  if(base_name == "InnerPortMSIUncached")  descriptor = new TypeInnerPortMSIUncached(type_name);
  if(base_name == "InnerPortMSIBroadcast") descriptor = new TypeInnerPortMSIBroadcast(type_name);
  if(base_name == "CoreInterfaceMSI")      descriptor = new TypeCoreInterfaceMSI(type_name);
  if(base_name == "CoherentCacheNorm")     descriptor = new TypeCoherentCacheNorm(type_name);
  if(base_name == "CoherentL1CacheNorm")   descriptor = new TypeCoherentL1CacheNorm(type_name);
  if(base_name == "SimpleMemoryModel")     descriptor = new TypeSimpleMemoryModel(type_name);
  if(base_name == "IndexNorm")             descriptor = new TypeIndexNorm(type_name);
  if(base_name == "IndexSkewed")           descriptor = new TypeIndexSkewed(type_name);
  if(base_name == "IndexRandom")           descriptor = new TypeIndexRandom(type_name);
  if(base_name == "ReplaceFIFO")           descriptor = new TypeReplaceFIFO(type_name);
  if(base_name == "ReplaceLRU")            descriptor = new TypeReplaceLRU(type_name);
  if(base_name == "DelayL1")               descriptor = new TypeDelayL1(type_name);
  if(base_name == "DelayCoherentCache")    descriptor = new TypeDelayCoherentCache(type_name);
  if(base_name == "DelayMemory")           descriptor = new TypeDelayMemory(type_name);

  if(nullptr == descriptor) {
    std::cerr << "[Decode] Fail to match `" << base_name << "' with a known base type." << std::endl;
    return false;
  }
  if(!descriptor->set(params)) return false;
  if(!add(type_name, descriptor)) return false;
  descriptor->emit_header();
  codegendb.type_declarations.push_back(descriptor);
  return true;
}

void Description::emit_header() { codegendb.add_header("cache/cache.hpp"); }

bool TypeMetadataMSI::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, AW)) return false; it++;
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, TOfst)) return false; it++;
  return true;
}

void TypeMetadataMSI::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << AW << "," << IW << "," << TOfst << "> " << this->name << ";" << std::endl;
}

void TypeMetadataMSI::emit_header() { codegendb.add_header("cache/msi.hpp"); }

bool TypeData64B::set(std::list<std::string> &values) {
  if(values.empty()) return true;
  std::cerr << "[No Paramater] " << tname << " supports no parameter!" << std::endl;
  return false;
}

void TypeData64B::emit(std::ofstream &file) {
  if(!this->name.empty())
    file << "typedef " << tname << " " << this->name << ";" << std::endl;
}

bool TypeCacheArrayNorm::set(std::list<std::string> &values) {
  if(values.size() != 4) {
    std::cerr << "[Mismatch] " << tname << " needs 4 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, NW)) return false; it++;
  MT = *it; if(!this->check(tname, "MT", *it, "CMMetadataBase", false)) return false; it++;
  DT = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  return true;
}

void TypeCacheArrayNorm::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << NW << "," << MT << "," << DT << "> " << this->name << ";" << std::endl;
}

bool TypeCacheSkewed::set(std::list<std::string> &values) {
  if(values.size() != 9) {
    std::cerr << "[Mismatch] " << tname << " needs 9 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, NW)) return false; it++;
  if(!codegendb.parse_int(*it, P)) return false; it++;
  MT  = *it; if(!this->check(tname, "MT", *it, "CMMetadataBase", false)) return false; it++;
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  IDX = *it; if(!this->check(tname, "IDX", *it, "IndexFuncBase", false)) return false; it++;
  RPC = *it; if(!this->check(tname, "RPC", *it, "ReplaceFuncBase", false)) return false; it++;
  DLY = *it; if(!this->check(tname, "DLY", *it, "DelayBase", true)) return false; it++;
  if(!codegendb.parse_bool(*it, EnMon)) return false; it++;
  return true;
}
 
void TypeCacheSkewed::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << NW << "," << P << "," << MT << "," << DT << "," << IDX << "," << RPC << "," << DLY << "," << EnMon << "> " << this->name << ";" << std::endl;
}

bool TypeCacheNorm::set(std::list<std::string> &values) {
  if(values.size() != 8) {
    std::cerr << "[Mismatch] " << tname << " needs 8 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, NW)) return false; it++;
  MT  = *it; if(!this->check(tname, "MT", *it, "CMMetadataBase", false)) return false; it++;
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  IDX = *it; if(!this->check(tname, "IDX", *it, "IndexFuncBase", false)) return false; it++;
  RPC = *it; if(!this->check(tname, "RPC", *it, "ReplaceFuncBase", false)) return false; it++;
  DLY = *it; if(!this->check(tname, "DLY", *it, "DelayBase", true)) return false; it++;
  if(!codegendb.parse_bool(*it, EnMon)) return false; it++;
  return true;
}
 
void TypeCacheNorm::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << NW << "," << MT << "," << DT << "," << IDX << "," << RPC << "," << DLY << "," << EnMon << "> " << this->name << ";" << std::endl;
}

bool TypeOuterPortMSIUncached::set(std::list<std::string> &values) {
  if(values.size() != 2) {
    std::cerr << "[Mismatch] " << tname << " needs 2 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  MT  = *it; if(!this->check(tname, "MT", *it, "MetadataMSIBase", false)) return false; it++;
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  return true;
}
  
void TypeOuterPortMSIUncached::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << MT << "," << DT << "> " << this->name << ";" << std::endl;
}  

void TypeOuterPortMSIUncached::emit_header() { codegendb.add_header("cache/msi.hpp"); }

bool TypeOuterPortMSI::set(std::list<std::string> &values) {
  if(values.size() != 2) {
    std::cerr << "[Mismatch] " << tname << " needs 2 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  MT  = *it; if(!this->check(tname, "MT", *it, "MetadataMSIBase", false)) return false; it++;
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  return true;
}
  
void TypeOuterPortMSI::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << MT << "," << DT << "> " << this->name << ";" << std::endl;
}  

void TypeOuterPortMSI::emit_header() { codegendb.add_header("cache/msi.hpp"); }

bool TypeInnerPortMSIUncached::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  MT  = *it; if(!this->check(tname, "MT", *it, "MetadataMSIBase", false)) return false; it++;
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  if(!codegendb.parse_bool(*it, isLLC)) return false; it++;
  return true;
}
  
void TypeInnerPortMSIUncached::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << MT << "," << DT << "," << isLLC << "> " << this->name << ";" << std::endl;
}    

void TypeInnerPortMSIUncached::emit_header() { codegendb.add_header("cache/msi.hpp"); }

bool TypeInnerPortMSIBroadcast::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  MT  = *it; if(!this->check(tname, "MT", *it, "MetadataMSIBase", false)) return false; it++;
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  if(!codegendb.parse_bool(*it, isLLC)) return false; it++;
  return true;
}
  
void TypeInnerPortMSIBroadcast::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << MT << "," << DT << "," << isLLC << "> " << this->name << ";" << std::endl;
}    

void TypeInnerPortMSIBroadcast::emit_header() { codegendb.add_header("cache/msi.hpp"); }

bool TypeCoreInterfaceMSI::set(std::list<std::string> &values) {
  if(values.size() != 4) {
    std::cerr << "[Mismatch] " << tname << " needs 4 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  MT  = *it; if(!this->check(tname, "MT", *it, "MetadataMSIBase", false)) return false; it++;
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  if(!codegendb.parse_bool(*it, enableDelay)) return false; it++;
  if(!codegendb.parse_bool(*it, isLLC)) return false; it++;
  return true;
}
  
void TypeCoreInterfaceMSI::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << MT << "," << DT << "," << enableDelay << "," << isLLC << "> " << this->name << ";" << std::endl;
}    

void TypeCoreInterfaceMSI::emit_header() { codegendb.add_header("cache/msi.hpp"); }

void TypeCoherentCacheBase::emit_header() { codegendb.add_header("cache/coherence.hpp"); }

bool TypeCoherentCacheNorm::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  CacheT = *it; if(!this->check(tname, "CacheT", *it, "CacheBase", false)) return false; it++;
  OuterT = *it; if(!this->check(tname, "OuterT", *it, "OuterCohPortBase", false)) return false; it++;
  InnerT = *it; if(!this->check(tname, "InnerT", *it, "InnerCohPortBase", false)) return false; it++;
  return true;
}
  
void TypeCoherentCacheNorm::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << CacheT << "," << OuterT << "," << InnerT << "> " << this->name << ";" << std::endl;
}

bool TypeCoherentL1CacheNorm::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  CacheT = *it; if(!this->check(tname, "CacheT", *it, "CacheBase", false)) return false; it++;
  OuterT = *it; if(!this->check(tname, "OuterT", *it, "OuterCohPortBase", false)) return false; it++;
  CoreT = *it;  if(!this->check(tname, "CoreT", *it, "CoreInterfaceBase", false)) return false; it++;
  return true;
}
  
bool TypeSimpleMemoryModel::set(std::list<std::string> &values) {
  if(values.size() != 2) {
    std::cerr << "[Mismatch] " << tname << " needs 2 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  DT  = *it; if(!this->check(tname, "DT", *it, "CMDataBase", true)) return false; it++;
  DLY = *it; if(!this->check(tname, "DLY", *it, "DelayBase", true)) return false; it++;
  return true;
}

void TypeSimpleMemoryModel::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << DT << "," << DLY << "> " << this->name << ";" << std::endl;
}

void TypeSimpleMemoryModel::emit_header() { codegendb.add_header("cache/memory.hpp"); }

void TypeCoherentL1CacheNorm::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << CacheT << "," << OuterT << "," << CoreT << "> " << this->name << ";" << std::endl;
}

void TypeIndexFuncBase::emit_header() { codegendb.add_header("cache/index.hpp"); }

bool TypeIndexNorm::set(std::list<std::string> &values) {
  if(values.size() != 2) {
    std::cerr << "[Mismatch] " << tname << " needs 2 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, IOfst)) return false; it++;
  return true;
}
  
void TypeIndexNorm::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << IOfst << "> " << this->name << ";" << std::endl;
}  

bool TypeIndexSkewed::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, IOfst)) return false; it++;
  if(!codegendb.parse_int(*it, P)) return false; it++;
  return true;
}
  
void TypeIndexSkewed::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << IOfst << "," << P << "> " << this->name << ";" << std::endl;
}  

bool TypeIndexRandom::set(std::list<std::string> &values) {
  if(values.size() != 2) {
    std::cerr << "[Mismatch] " << tname << " needs 2 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, IOfst)) return false; it++;
  return true;
}
  
void TypeIndexRandom::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << IOfst << "> " << this->name << ";" << std::endl;
}  

void TypeReplaceFuncBase::emit_header() { codegendb.add_header("cache/replace.hpp"); }

bool TypeReplaceFIFO::set(std::list<std::string> &values) {
  if(values.size() != 2) {
    std::cerr << "[Mismatch] " << tname << " needs 2 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, NW)) return false; it++;
  return true;
}
  
void TypeReplaceFIFO::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << NW << "> " << this->name << ";" << std::endl;
}  

bool TypeReplaceLRU::set(std::list<std::string> &values) {
  if(values.size() != 2) {
    std::cerr << "[Mismatch] " << tname << " needs 2 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, IW)) return false; it++;
  if(!codegendb.parse_int(*it, NW)) return false; it++;
  return true;
}
  
void TypeReplaceLRU::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << IW << "," << NW << "> " << this->name << ";" << std::endl;
}  

void TypeDelayBase::emit_header() { codegendb.add_header("cache/delay.hpp"); }

bool TypeDelayL1::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, dhit)) return false; it++;
  if(!codegendb.parse_int(*it, dreplay)) return false; it++;
  if(!codegendb.parse_int(*it, dtran)) return false; it++;
  return true;
}

void TypeDelayL1::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << dhit << "," << dreplay << "," << dtran << "> " << this->name << ";" << std::endl;
}

bool TypeDelayCoherentCache::set(std::list<std::string> &values) {
  if(values.size() != 3) {
    std::cerr << "[Mismatch] " << tname << " needs 3 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, dhit)) return false; it++;
  if(!codegendb.parse_int(*it, dtranUp)) return false; it++;
  if(!codegendb.parse_int(*it, dtranDown)) return false; it++;
  return true;
}

void TypeDelayCoherentCache::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << dhit << "," << dtranUp << "," << dtranDown << "> " << this->name << ";" << std::endl;
}

bool TypeDelayMemory::set(std::list<std::string> &values) {
  if(values.size() != 1) {
    std::cerr << "[Mismatch] " << tname << " needs 1 parameters!" << std::endl;
    return false;
  }
  auto it = values.begin();
  if(!codegendb.parse_int(*it, dtran)) return false; it++;
  return true;
}

void TypeDelayMemory::emit(std::ofstream &file) {
  file << "typedef " << tname << "<" << dtran << "> " << this->name << ";" << std::endl;
}
