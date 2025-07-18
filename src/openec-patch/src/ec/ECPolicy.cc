#include "ECPolicy.hh"

// ECPolicy::ECPolicy(string id, string classname, int n, int k, int w, bool locality, int opt, vector<string> param) {
ECPolicy::ECPolicy(string id, string classname, int n, int k, int w, int opt, vector<string> param)
{
  _id = id;
  _classname = classname;
  _n = n;
  _k = k;
  _w = w;
  //  _locality = locality;
  _opt = opt;
  _param = param;
}

ECBase *ECPolicy::createECClass()
{
  ECBase *toret;
  if (_classname == "RSCONV")
  {
    //    toret = new RSCONV(_n, _k, _w, _locality, _opt, _param);
    toret = new RSCONV(_n, _k, _w, _opt, _param);
  }
  else if (_classname == "HHXORPlus")
  {
    //    toret = new HHXORPlus(_n, _k, _w, _locality, _opt, _param);
    toret = new HHXORPlus(_n, _k, _w, _opt, _param);
  }
  else if (_classname == "HTEC")
  {
    //    toret = new HTEC(_n, _k, _w, _locality, _opt, _param);
    toret = new HTEC(_n, _k, _w, _opt, _param);
  }
  else if (_classname == "ETRSConv")
  {
    //    toret = new ETRSConv(_n, _k, _w, _locality, _opt, _param);
    toret = new ETRSConv(_n, _k, _w, _opt, _param);
  }
  else if (_classname == "Clay")
  {
    //    toret = new Clay(_n, _k, _w, _locality, _opt, _param);
    toret = new Clay(_n, _k, _w, _opt, _param);
  }
  else if (_classname == "LESS")
  {
    //    toret = new LESS(_n, _k, _w, _locality, _opt, _param);
    toret = new LESS(_n, _k, _w, _opt, _param);
  }
  else
  {
    cout << "unrecognized code, use default RSCONV" << endl;
    //    toret = new RSCONV(_n, _k, _w, _locality, _opt, _param);
    toret = new RSCONV(_n, _k, _w, _opt, _param);
  }
  return toret;
}

string ECPolicy::getPolicyId()
{
  return _id;
}

int ECPolicy::getN()
{
  return _n;
}

int ECPolicy::getK()
{
  return _k;
}

int ECPolicy::getW()
{
  return _w;
}

bool ECPolicy::getLocality()
{
  return _locality;
}

int ECPolicy::getOpt()
{
  return _opt;
}

string ECPolicy::getClassName()
{
  return _classname;
}

vector<string> ECPolicy::getParams()
{
  return _param;
}
