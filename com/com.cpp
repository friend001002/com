#include <iostream>
#include <atomic>

//#include <winerror.h>
#include <objbase.h>

using namespace std;

const IID IID_I_x = { 1, 1, 1, {1, 2, 3, 4, 5, 6, 7, 8} };
const IID IID_I_y = { 2, 2, 2, {1, 2, 3, 4, 5, 6, 7, 8} };
const IID IID_I_z = { 3, 3, 3, {1, 2, 3, 4, 5, 6, 7, 8} };

struct I_x : IUnknown
{
  virtual void __stdcall I_x_foo() = 0;
};

struct I_y : IUnknown
{
  virtual void __stdcall I_y_foo() = 0;
};

struct I_z : IUnknown
{
  virtual void __stdcall I_z_foo() = 0;
};

ostream& operator<<(ostream& os, const IID iid)
{
  os << hex << '{' << static_cast<uint32_t>(iid.Data1)    << '-' 
                   << static_cast<uint32_t>(iid.Data2)    << '-'
                   << static_cast<uint32_t>(iid.Data3)    << '-'
                   << static_cast<uint32_t>(iid.Data4[0]) << '-'
                   << static_cast<uint32_t>(iid.Data4[1]) << '-'
                   << static_cast<uint32_t>(iid.Data4[2]) << '-'
                   << static_cast<uint32_t>(iid.Data4[3]) << '-'
                   << static_cast<uint32_t>(iid.Data4[4]) << '-'
                   << static_cast<uint32_t>(iid.Data4[5]) << '-'
                   << static_cast<uint32_t>(iid.Data4[6]) << '-'
                   << static_cast<uint32_t>(iid.Data4[7]) 
            << "}\n";

  return os;
}

class C_a : public I_x, public I_y
{
  public:

  C_a() : refs_(0)
  {
    cout << "C_a constructor\n";
  }

  virtual ~C_a()
  {
    cout << "C_a destructor\n";
  }

  void __stdcall I_x_foo() override
  {
    cout << "I_x_foo\n";
  }

  void __stdcall I_y_foo() override
  {
    cout << "I_y_foo\n";
  }

  private:

  HRESULT __stdcall QueryInterface(const IID& iid, void **ppv) override
  {
    if (IID_IUnknown == iid)
    {
      cout << "Query: IUnknown\n";
      *ppv = static_cast<I_x*>(this);
    }
    else if (IID_I_x == iid)
    {
      cout << "Query: I_x\n";
      *ppv = static_cast<I_x*>(this);
    }
    else if (IID_I_y == iid)
    {
      cout << "Query: I_y\n";
      *ppv = static_cast<I_y*>(this);
    }
    else
    {
      cerr << "Query: component C_a doesn't contain interface " << iid << endl;
      *ppv = nullptr;

      return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
  }

  ULONG __stdcall AddRef() override
  {
    refs_++;
    cout << "C_a: AddRef = " << refs_ << endl;
    return refs_;
  }

  ULONG __stdcall Release() override
  {
    if (0 == refs_)
    {
      cerr << "C_a: Release: reference count was already zero!\n";
      return refs_;
    }
    else
    {
      refs_--;
      cout << "C_a: Release = " << refs_ << endl;

      if (0 == refs_)
      {
        delete this;
        return refs_;
      }

      return refs_;
    }
  }
  
  atomic<uint16_t> refs_;
};

IUnknown* CreateInstance()
{
  IUnknown* iu = static_cast<I_x*>(new C_a);

  iu->AddRef();

  return iu;
}

int main()
{
  HRESULT hr;

  IUnknown *iu = CreateInstance();

  I_x *ix = nullptr;
  hr = iu->QueryInterface(IID_I_x, reinterpret_cast<void**>(&ix));

  if (SUCCEEDED(hr))
  {
    ix->I_x_foo();

    I_x *ix_ix = nullptr;
    hr = ix->QueryInterface(IID_I_x, reinterpret_cast<void**>(&ix_ix));

    if (SUCCEEDED(hr))
    {
      cout << "ix == ix_ix " << (ix == ix_ix) 
           << endl;

      ix_ix->Release();
    }
    else
    {
      cerr << "Failed to get I_x via I_x\n";
    }

    ix->Release();
  }
  else
  {
    cerr << "Failed to get I_x via IUnknown\n";
  }

  I_y *iy = nullptr;
  hr = iu->QueryInterface(IID_I_y, reinterpret_cast<void**>(&iy));

  if (SUCCEEDED(hr))
  {
    IUnknown *iu_iu = nullptr;
    hr = iy->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&iu_iu));

    if (SUCCEEDED(hr))
    {
      cout << "iu == iu_iu " << (iu == iu_iu) 
           << endl;

      iu_iu->Release();
    }
    else
    {
      cerr << "Failed to get IUnknown via I_y\n";
    }

    iy->I_y_foo();
    iy->Release();
  }
  else
  {
    cerr << "Failed to get I_y via IUnknown\n";
  }

  I_z *iz = nullptr;
  hr = iu->QueryInterface(IID_I_z, reinterpret_cast<void**>(&iz));

  if (SUCCEEDED(hr))
  {
    iz->I_z_foo();
    iz->Release();
  }
  else
  {
    cerr << "Failed to get I_z via IUnknown\n";
  }

  iu->Release();

  cin.get();

  return 0;
}
