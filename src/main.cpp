#include "wx.h"

class MyApp : public wxApp
{

public:
	virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title);

//private:
//	wxDECLARE_EVENT_TABLE();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	MyFrame *frame = new MyFrame("Minimal wxWidgets App");

	frame->Show(true);

	return true;
}

MyFrame::MyFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{

}