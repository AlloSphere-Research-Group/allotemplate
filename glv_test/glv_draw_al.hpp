#pragma once

#include "al/core.hpp"

#include "al/glv/glv_core.h" // GLV, View
#include "al/glv/glv_icon.h" // Icon (Check, Cross, Rectangle)
#include "al/glv/glv_draw.h" // GraphicsData
#include "al/glv/glv_buttons.h"
#include "al/glv/glv_sliders.h"
#include "al/glv/glv_font.h" 
#include "al/glv/glv_layout.h" // Divider, Table
#include "al/glv/glv_textview.h"
#include "al/glv/glv_widget.h"

#include <cmath>
#include <iostream>

namespace glv {

#define GLV_INDEX GL_UNSIGNED_INT

struct GraphicsHolder {
	al::Graphics* mGraphics;
	float w;
	float h;
	void set(al::Graphics& g) {
		mGraphics = &g;
	} 
	al::Graphics& get() {
		return *mGraphics;
	}
	void size(float width, float height) {
		w = width;
		h = height;
	}
};

GraphicsHolder& graphicsHolder() {
	static GraphicsHolder graphicsHolder;
	return graphicsHolder;
}

const double C_PI = 4. * atan(1.);
const double C_2PI = 2. * C_PI;

/// Returns closest pixel coordinate
int pix(float v){ return v>=0 ? (int)(v+0.5f) : (int)(v-0.5f); }
/// Returns center of closest pixel coordinate
float pixc(float v){ return pix(v) + 0.5f; }

void rectangle(float l, float t, float r, float b)
{
	static al::VAOMesh mesh;
	auto& gh = graphicsHolder();

	float x = l;
	float y = gh.h - b;
	float w = r - l;
	float h = b - t;
	mesh.reset();
	addRect(mesh, x, y, w, h);
	mesh.update();
	gh.get().draw(mesh);
}

void frame(float l, float t, float r, float b)
{
	static al::VAOMesh mesh;
	auto& gh = graphicsHolder();
	float x = l;
	float y = gh.h - b;
	float w = r - l;
	float h = b - t;
	mesh.reset();
	mesh.primitive(al::Mesh::LINE_STRIP);
	mesh.vertex(x, y);
	mesh.vertex(x + w, y);
	mesh.vertex(x + w, y + h);
	mesh.vertex(x, y + h);
	mesh.vertex(x, y);
	mesh.update();
	gh.get().draw(mesh);
}

void line(float x0, float y0, float x1, float y1)
{
	static al::VAOMesh mesh;
	auto& gh = graphicsHolder();
	mesh.reset();
	mesh.primitive(al::Mesh::LINES);
	mesh.vertex(x0, gh.h - y0);
	mesh.vertex(x1, gh.h - y1);
	mesh.update();
	gh.get().draw(mesh);
}

void grid (
	float l, float t, float w, float h,
	float divx, float divy,
	bool incEnds=true
)
{
	static al::VAOMesh mesh;
	auto& gh = graphicsHolder();
	mesh.reset();
	mesh.primitive(al::Mesh::LINES);
	double inc, r=l+w, b=t+h;

	if(divy > 0 && h>0){
		inc = (double)h/(double)divy;
		double i = incEnds ? t-0.0001 : t-0.0001+inc;
		double e = incEnds ? b : b-inc;
		for(; i<e; i+=inc) {
			mesh.vertex(l, gh.h - i);
			mesh.vertex(r, gh.h - i);
		}
	}

	if(divx > 0 && w>0){
		inc = (double)w/(double)divx;
		double i = incEnds ? l-0.0001 : l-0.0001+inc;
		double e = incEnds ? r : r-inc;
		for(; i<e; i+=inc) {
			mesh.vertex(i, gh.h - t);
			mesh.vertex(i, gh.h - b);
		}
	}
	mesh.update();
	gh.get().draw(mesh);
}

void text (
	const char * s,
	float l=0, float t=0,
	unsigned fontSize=8, float lineSpacing=1.5, unsigned tabSpaces=4
)
{
	Font f;
	f.size(fontSize);
	f.lineSpacing(lineSpacing);
	f.tabSpaces(tabSpaces);
	f.render(s, l, t, 0);
}

void drawContext (
	float tx, float ty, View * v, float& cx, float& cy, View *& c
)
{
	auto& gh = graphicsHolder();
	auto& g = gh.get();
	cx += tx; cy += ty; // update absolute coordinates of drawing context
	g.loadIdentity(); // clear model matrix (assumed set already)
	// pix: round position to nearest pixel coordinates
	// (-) on y: glv has origin at top left,
	// but al_lib 2D camera has it at bottom left
	g.translate(pix(cx), -pix(cy)); 
	c = v;
}

void computeCrop(
	std::vector<Rect>& cr, int lvl, space_t ax, space_t ay, View * v
)
{
	if (v->enabled(CropChildren)) {
		cr[lvl].set(ax, ay, v->w, v->h);	// set absolute rect
		// get intersection with myself and previous level
		if (lvl>0) {
			Rect r = cr[lvl];
			r.resizeEdgesBy(-1);	// shrink area to save the borders
			r.intersection(cr[lvl-1], cr[lvl]);
		}
	}
	// if no child cropping, then inherit previous level's crop rect
	else{ cr[lvl] = cr[lvl-1]; }
}

// Views are drawn depth-first from leftmost to rightmost sibling
void GLV::drawWidgets(unsigned int ww, unsigned int wh, double dsec) {
	auto& gh = graphicsHolder();
	auto& g = gh.get();

	// TODO: Perhaps the View tree should be serialized into a separate list
	//		used for rendering?
	//		This will permit a user to change the graph structure during a draw 
	//		callback. Currently if this is attempted, we crash and burn.

	// Render all primitives at integer positions, ref: OpenGL Redbook
	// NOTE: This is a comprise to get almost pixel-perfection for both lines 
	// (half-integers) and polygons (integers). We'll do it "by hand" due to all
	// the exceptions and to get exact pixel-perfect accuracy.

	float cx = 0, cy = 0; // drawing context absolute position
	View * const root = this;
	View * cv = root;

	// The crop region is the intersection of all parent rects up to the top 
	// view. The intersections also need to be done in absolute coordinates.	
	std::vector<Rect> cropRects(16, Rect(ww, wh));	// index is hierarchy level
	int lvl = 0;	// start at root = 0

	// Animate all the views
	struct AnimateViews : public TraversalAction{
		AnimateViews(double dt_): dt(dt_){}
		virtual bool operator()(View * v, int depth){
			if(v->enabled(Animate)) v->onAnimate(dt);
			return true;
		}
		double dt;
	} animateViews(dsec);
	traverseDepth(animateViews);

	graphicsData().reset();
	doDraw(*this);

	g.scissorTest(true);

	while(true){

		cv->onDataModelSync();		// update state based on attached model variables
		cv->rectifyGeometry();

		// find the next view to draw

		// go to child node if exists and I'm drawable
		if(cv->child && cv->visible()){
			drawContext(cv->child->l, cv->child->t, cv->child, cx, cy, cv);
			computeCrop(cropRects, ++lvl, cx, cy, cv);
		}

		// go to sibling node if exists
		else if(cv->sibling){
			drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
			computeCrop(cropRects, lvl, cx, cy, cv);
		}

		// retrace upwards until a parent's sibling is found
		else{
			while(cv != root && cv->sibling == 0){
				drawContext(-cv->l, -cv->t, cv->parent, cx, cy, cv);
				lvl--;
			}

			if(cv->sibling){
				drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
				computeCrop(cropRects, lvl, cx, cy, cv);
			}
			else break; // break the loop when the traversal returns to the root
		}
		
		// draw current view
		if(cv->visible()){
			Rect r = cropRects[lvl-1];	// cropping region comes from parent context
			if(cv->enabled(CropSelf)) r.intersection(Rect(cx, cy, cv->w, cv->h), r); // crop my own draw?

			// bypass if drawing area outside of crop region
			if(r.h<=0.f || r.w <= 0.f) continue;

			// scissor coord has origin at bottom left
			int sx = pix(r.l);
			int sy = wh - (pix(r.t) + pix(r.h)) + 0.99;
			int sw = pix(r.w);
			int sh = r.h + 0.5;
			if(sy < 0) sy=0;

			// glScissor takes size in framebuffer dimension
			g.scissor(
				sx * g.window().x_highres(),
				sy * g.window().y_highres(),
				sw * g.window().x_highres(),
				sh * g.window().y_highres()
			);

			graphicsData().reset();
			cv->doDraw(*this);
		}
	}

	g.scissorTest(false);
}

void View::doDraw(GLV& glv){
	al::Graphics& g = graphicsHolder().get();

	if(enabled(DrawBack)){
		glv::Color& colbg = colors().back;
		g.uniformColor(colbg.r, colbg.g, colbg.b, colbg.a);
		rectangle(0, 0, w, h);
	}

	bool drawNext = true;
	DrawHandlers::iterator it = mDrawHandlers.begin();
	while(it != mDrawHandlers.end()){
		DrawHandlers::iterator itnext = ++it; --it;
		glv.graphicsData().reset();
		drawNext = (*it)->onDraw(*this, glv);
		if(!drawNext) break;
		it = itnext;
	}

	if(drawNext){
		glv.graphicsData().reset();
		onDraw(glv);
	}

	if(enabled(DrawBorder)){
		float borderWidth = 1.0;

		// double border thickness if focused
		if(enabled(Focused) && enabled(FocusHighlight)){
			borderWidth *= 2;
		}

		// g.lineWidth(borderWidth); // disabled in >gl3
		glv::Color colborder = colors().border;
		g.uniformColor(colborder.r, colborder.g, colborder.b, colborder.a);
		frame(1.0, 1.0, pix(w)-0.5, pix(h)-0.5);
		frame(0.5, 0.5, pix(w), pix(h));
		// const float ds = 0.5; // OpenGL suggests 0.375, but smears with AA enabled
		// frame(ds, ds, pix(w)-ds, pix(h)-ds);
		// frame(0,0, pix(w)-ds, pix(h)-ds); // hack to give bevelled look
	}
}

void Widget::drawGrid(GraphicsData& g){
}

void Widget::drawSelectionBox(){
}

void Widget::onDraw(GLV& g){
}

void Sliders::onDraw(GLV& glv) {
	auto& gh = graphicsHolder();
	auto& g = gh.get();
	Widget::onDraw(glv);

	float x=paddingX(), xd=dx(), yd=dy();

	// TODO: fix padding in orientation direction

	if(vertOri()){
		for(int i=0; i<sizeX(); ++i){
		
			float y=paddingY();
		
			for(int j=0; j<sizeY(); ++j){
				int ind = index(i,j);
				if(isSelected(i,j)) g.uniformColor(
					colors().fore.r,
					colors().fore.g,
					colors().fore.b
				);
				else g.uniformColor(
					colors().fore.r,
					colors().fore.g,
					colors().fore.b,
					colors().fore.a*0.5f
				);

				float v01 = to01(getValue(ind));
				//float y0 = to01(0)*(yd - paddingY()*2);
				float y0 = to01(0)*yd;
				//rect(x + x0, y, f*xd+x, y+yd-padding());
				
				rectangle(x, y + (yd-v01*yd), x+xd-paddingX()*2, y + (yd-y0));

				// if zero line showing
				if(max()>0 && min()<0){
					g.uniformColor(
						colors().border.r,
						colors().border.g,
						colors().border.b
					);
					float linePos = pixc(y+yd-y0);
					line(x, linePos, x+xd, linePos);
				}
				y += yd;
			}
			x += xd;	
		}
	}
	else{
		for(int i=0; i<sizeX(); ++i){
		
			float y=paddingY();
		
			for(int j=0; j<sizeY(); ++j){
				int ind = index(i,j);
				if(isSelected(i,j)) g.uniformColor(
					colors().fore.r,
					colors().fore.g,
					colors().fore.b
				);
				else g.uniformColor(
					colors().fore.r,
					colors().fore.g,
					colors().fore.b,
					colors().fore.a*0.5f
				);

				float v01 = to01(getValue(ind));
				float x0 = to01(0)*xd;
				rectangle(x + x0, y, v01*xd+x, y+yd-paddingY()*2);

				// if zero line showing
				if(max()>0 && min()<0){
					g.uniformColor(
						colors().border.r,
						colors().border.g,
						colors().border.b
					);
					float linePos = pixc(x+x0);
					line(linePos, y, linePos, y+yd);
				}
				y += yd;
			}
			x += xd;
		}
	}
}

void Font::render(GraphicsData& gd, const char * v, float x, float y, float z) const {

}

void SliderRange::onDraw(GLV& g) {

}

void Slider2D::onDraw(GLV& g) {

}



// -----------------------------------------------------------------------------
#ifdef DONT_COMMENT_OUT

void Check::draw(float l, float t, float r, float b){
	shape(LineStrip, l,0.5f*(t+b), l+(r-l)*0.3f,b, r,t);
};

void Cross::draw(float l, float t, float r, float b){
	shape(Lines, l,t, r,b, l,b, r,t);
};

void Rectangle::draw(float l, float t, float r, float b){
	shape(TriangleStrip, l,t, l,b, r,t, r,b);
};

void Buttons::onDraw(GLV& g){
	Widget::onDraw(g);

	using namespace glv::draw;

	float xd = dx();
	float yd = dy();
	float padx = paddingX();
	float pady = paddingY();
	color(colors().fore);
	
	// TODO: small buttons hard to see when not antialiased
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	//enable(PolygonSmooth);
	
	stroke(1);

	for(int i=0; i<sizeX(); ++i){
		float x = xd*i + padx;

		for(int j=0; j<sizeY(); ++j){
			float y = yd*j + pady;
			if(getValue(i,j)){	if(mSymOn ) mSymOn (x, y, x+xd-padx*2, y+yd-pady*2); }
			else{				if(mSymOff) mSymOff(x, y, x+xd-padx*2, y+yd-pady*2); }
		}		
	}
	
	//disable(PolygonSmooth);
}

void Slider2D::onDraw(GLV& g){

	if(!g.mouse().isDown() && enabled(Momentary)) setValueMid();

	using namespace glv::draw;
	float sz = knobSize();	// size of indicator block
	float sz2 = sz * 0.5f;
	float posX, posY;
	
	if(mConstrainKnob){
		posX = sz2 + (w - sz) * to01(getValue(0));
		posY = sz2 + (h - sz) * (1.f - to01(getValue(1)));
	}
	else{
		posX = w * to01(getValue(0));
		posY = h * (1.f - to01(getValue(1)));	
	}
	
	color(colors().fore);
	mKnobSym((posX - sz2), (posY - sz2), (posX + sz2), (posY + sz2));

}


void SliderRange::onDraw(GLV& g){
	using namespace glv::draw;

	float v1 = to01(getValue(0));
	float v2 = to01(getValue(1));
	if(v2<v1){ float t=v1; v1=v2; v2=t; }
	
	// prevent degeneracy
	float p1 = 1./((w>h)?w:h); // 1 pixel
	if(v2-v1 <= p1){ v1-=p1*0.5; v2+=p1*0.5; }

	color(colors().fore);
	if(w>h){	// horizontal
		//rectTrunc<2,2,2,2>(v1*w,0, v2*w,h);
		float x1 = v1*w;
		float x2 = v2*w;
		rectangle(x1+2,0, x2-2,h);
		x1 = pixc(x1);
		x2 = pixc(x2)-1;
		shape(Lines, x1,0, x1,h, x2,0, x2,h);
	}
	else{
//		rectTrunc<2,2,2,2>(0,v1*h, w,v2*h);
//		rectTrunc<2,2,2,2>(0,h - v2*h, w, h - v1*h); // REV: flip y

		float y1 = h-v1*h;
		float y2 = h-v2*h;
		rectangle(0,y2+2, w,y1-2);
		y1 = pixc(y1)-1;
		y2 = pixc(y2);
		shape(Lines, 0,y1, w,y1, 0,y2, w,y2);
	}
}




void Font::render(GraphicsData& gd, const char * v, float x, float y, float z) const{
	using namespace glv::draw;

	gd.reset();

	float sx = mScaleX;
	float sy = mScaleY;
	float tx = x;
	float ty = y;
	//float tz = z;
	//float sh = -0.5*sy; // TODO: shear needs to be done an a per-line basis
	//float sh = 0;
	
	//tx=ty=tz=0;

	struct RenderText : public TextIterator{
		RenderText(const Font& f_, const char *& s_, GraphicsData& g_, float tx_, float ty_, float sx_, float sy_)
		: TextIterator(f_,s_), g(g_), tx(tx_), ty(ty_), sx(sx_), sy(sy_){}
		bool onPrintable(char c){
			return addCharacter(g, c, pixc(tx+x*sx), pixc(ty+y*sy), sx, sy);
		}
		GraphicsData& g;
		float tx,ty,sx,sy;
	} renderText(*this, v, gd, tx,ty,sx,sy);

	renderText.run();
	
	paint(Lines, gd);
}



void Divider::onDraw(GLV& g){
	using namespace glv::draw;
	if(mStrokeWidth <= 0) return;

	lineWidth(mStrokeWidth);
	color(colors().fore);
	if(mIsVertical){
		float p = pixc(w/2);
		shape(Lines, p,0, p,h);		
	}
	else{
		float p = pixc(h/2);
		shape(Lines, 0,p, w,p);
	}
}

void Table::onDraw(GLV& g){
//	for(unsigned i=0; i<mColWs.size(); ++i) printf("%g ", mColWs[i]); printf("\n");
//	for(unsigned i=0; i<mRowHs.size(); ++i) printf("%g ", mRowHs[i]); printf("\n\n");

	using namespace glv::draw;
	if(enabled(DrawGrid)){
		color(colors().border);
		lineWidth(1);
		for(unsigned i=0; i<mCells.size(); ++i){
			space_t cl,ct,cr,cb;
			getCellDim(i, cl,ct,cr,cb);
			cl -= mPad1/2;
			cr += mPad1/2;
			ct -= mPad2/2;
			cb += mPad2/2;
			frame(cl,ct,cr,cb);
		}
	}
}

void Label::onDraw(GLV& g){
	using namespace glv::draw;
	lineWidth(stroke());
	color(colors().text);
	if(mVertical){ translate(0,h); rotate(0,0,-90); }
	font().render(
		g.graphicsData(),
		data().toString().c_str(),
		paddingX(),
		paddingY()
	);
	//scale(mSize, mSize);
	//text(value().c_str());
}


void TextView::onDraw(GLV& g){
	using namespace draw;

	float padX = paddingX();
	float padY = paddingY();
	float addY =-4;//was -2		// subtraction from top since some letters go above cap

	float tl = mPos * font().advance('M') + padX;
//	float tr = tl + font().advance('M');
	float tt = addY + padY;
	float tb = tt + fabs(font().descent()+font().cap()) - addY;
	float strokeWidth = 1;
	
	// draw selection
	if(textSelected()){
		float sl, sr;
		if(mSel>0){
			sl = tl;
			sr = sl + mSel*font().advance('M');
		}
		else{
			sr = tl;
			sl = sr + mSel*font().advance('M');
		}
		color(colors().selection);
		rectangle(sl, tt, sr, tb);
	}

	// draw cursor
	if(mBlink<0.5 && enabled(Focused)){
		stroke(1);
		color(colors().text);
		shape(Lines, pixc(tl), tt, pixc(tl), tb);
	}

	lineWidth(strokeWidth);
	color(colors().text);
//	font().render(mText.c_str(), pixc(padX), pixc(padY-1));
	font().render(g.graphicsData(), mText.c_str(), padX, padY-1);
}


ListView& ListView::fitExtent(){
	float maxw = 0;//, maxh = 0;
	int nitems = data().size();

	for(int i=0; i<nitems; ++i){
		float x = font().advance(data().at<std::string>(i).c_str());
		if(x > maxw) maxw = x;
	}
	extent(
		pix(data().size(0) * (maxw + paddingX()*2)),
		pix(data().size(1) * (font().cap() + font().descent() + paddingY()*2))
	);
	return *this;
}


void ListView::onDraw(GLV& g){

	using namespace glv::draw;

	Indexer idx(data().size(0), data().size(1));
	float dx_ = dx(0);
	float dy_ = dy(1);
	
	while(idx()){
		int ix = idx[0];
		int iy = idx[1];
		
		float px = dx_ * ix;
		float py = dy_ * iy;

		if(selectedX() == ix && selectedY() == iy){
			color(colors().selection);
			rectangle(px,py, px+dx_,py+dy_);
		}
		
		color(colors().text);
		lineWidth(1);
		
		//font().render(data().at<std::string>(ix,iy).c_str(), pixc(px+paddingX()), pixc(py+paddingY()));
		font().render(g.graphicsData(), data().at<std::string>(ix,iy).c_str(), px+paddingX(), py+paddingY());
	}
	
	Widget::onDraw(g);
}


void DropDown::onDraw(GLV& g){
	mBlink = 0.9;
	TextView::onDraw(g);
	
	color(colors().fore);
	float ds = 3;
	// triangleD(w - h + ds, ds, w-ds, h-ds);
}


void NumberDialers::fitExtent(){
	extent(
		pix(sizeX() * (paddingX()*2 + (numDigits() * font().advance('M'))) + 1),
		pix(sizeY() * (paddingY()*2 + font().cap()) + 1)
	);
//	print();
}


void NumberDialers::onDraw(GLV& g){ //printf("% g\n", value());
	using namespace glv::draw;

	fitExtent();

	float dxCell= dx();
	float dyCell= dy();
	float dxDig = font().advance('M');

//	View::enable(DrawSelectionBox);
//	View::enable(DrawGrid);

	// draw box at position (only if focused)
	if(enabled(Focused)){

		float x = dxCell*selectedX() + paddingX()/1 - 1;
		//float y = dyCell*selectedY() + paddingY()/2;
		float y = dyCell*(selectedY()+0.5);
		float ty= font().cap()/2. + 3;

//		color(colors().fore, colors().fore.a*0.4);
		color(colors().selection);
		//rectangle(bx + dig()*dxDig, by, bx + (dig()+1)*dxDig, by + dyCell-0.5f);
		rectangle(x + dig()*dxDig, y-ty, x + (dig()+1)*dxDig, y+ty);
	}

	drawSelectionBox();
	drawGrid(g.graphicsData());

	lineWidth(1);

	if(mTextEntryMode){
		mTextEntry.extent(dxCell, dyCell);
		mTextEntry.pos(dxCell*selectedX(), dyCell*selectedY());
	}

	for(int i=0; i<sizeX(); ++i){
		for(int j=0; j<sizeY(); ++j){

			float cx = dxCell*i;	// left edge of cell
			float cy = dyCell*j;	// top edge of cell

			// draw number
			long long vali = valInt(i,j);
			unsigned long absVal = vali < 0 ? -vali : vali;
			int msd = mNF;	// position from right of most significant digit

			if(absVal > 0){
				msd = (int)log10((double)absVal);
				int p = numDigits() - (mShowSign ? 2:1);
				msd = msd < mNF ? mNF : (msd > p ? p : msd);
			}

			if(mNI == 0) msd-=1;

			// Determine digit string
			char str[32];
			int ic = numDigits();
			str[ic] = '\0';
			for(int i=0; i<numDigits(); ++i) str[i]=' ';

			if(mShowSign && vali < 0) str[0] = '-';

			unsigned long long power = 1;
			bool drawChar = false; // don't draw until non-zero or past decimal point

			for(int i=0; i<=msd; ++i){
				char c = '0' + (absVal % (power*10))/power;
				power *= 10;
				if(c!='0' || i>=mNF) drawChar = true;
				--ic;
				if(drawChar) str[ic] = c;
			}

			// Draw the digit string
			float tx = int(cx + paddingX());
			float ty = int(cy + paddingY());

			if(vali || !dimZero()){
				color(colors().text);
			} else {
				color(colors().text.mix(colors().back, 0.8));
			}
		//	printf("%s\n", str);
//			font().render(g.graphicsData(), str, pixc(tx), pixc(ty));
//			if(mNF>0) font().render(g.graphicsData(), ".", pixc(dxDig*(mNI+numSignDigits()-0.5f) + tx), pixc(ty));
			font().render(g.graphicsData(), str, tx, ty);
			if(mNF>0) font().render(g.graphicsData(), ".", dxDig*(mNI+numSignDigits()-0.5f) + tx, ty);
		}
	}
}

void Scroll::onDraw(GLV& g){

	mSliderX.bringToFront();	// do not change order of these!
	mSliderY.bringToFront();
	mSliderXY.bringToFront();

	// hide scrollbars by default
	mSliderX.disable(Visible);
	mSliderY.disable(Visible);
	mSliderXY.disable(Visible);

	if(child == &mSliderX) return;

	Rect r = child->rect();
	
	r.w += paddingX()*2;
	r.h += paddingY()*2;

//		Rect r(0,0,0,0);
//		
//		{
//			View * c = child;
//			while(c){
//				if(c != &mSliderX && c != &mSliderY){
//					r.unionOf(*c, r);
//				}
//				c = c->sibling;
//			}
//		}
////		r.print();


	// slider units are in pixels

	float xpos = mSliderX.getValue(0);
	float ypos = mSliderY.getValue(1);
	child->pos(-xpos + paddingX(), ypos + paddingY());
	mSliderX.interval(0, r.width());
	mSliderY.interval(0,-r.height()); // use negative range so 0 is at top

	if(r.width() > width()){
		if(mMode & HORIZONTAL){
			mSliderX.enable(Visible);
			mSliderXY.enable(Visible);
		}
		// subtracting y slider width to fit content
		float sr = width() - mSliderY.width();
		mSliderX.endpoints(xpos, xpos+sr);
		mSliderX.jump(sr/(mSliderX.max()-mSliderX.min()));
	}

	if(r.height() > height()){
		if(mMode & VERTICAL){
			mSliderY.enable(Visible);
			mSliderXY.enable(Visible);
		}
		// subtracting x slider height to fit content
		float sr = height() - mSliderX.height();
		mSliderY.endpoints(ypos, ypos-sr);
		mSliderY.jump(sr/(mSliderY.max()-mSliderY.min()));
//		printf("%g %g\n", mSliderY.getValue(0), mSliderY.getValue(1));
	}
	
	if(mMode & ALWAYS){
		if(mMode & HORIZONTAL) mSliderX.enable(Visible);
		if(mMode & VERTICAL  ) mSliderY.enable(Visible);
	}
}


#endif
// -----------------------------------------------------------------------------

} // namespace glv

void al_draw_glv(
	glv::GLV& glv, al::Graphics& g,
	unsigned x, unsigned y, unsigned w, unsigned h, double dsec
)
{
	// g.lighting(false);
	g.depthTesting(false);
	g.blending(true);
	g.blendModeTrans();
	g.uniformColorMix(1);
	g.polygonMode(al::Graphics::FILL);
	g.cullFace(false);
	g.camera(
		al::Viewpoint::ORTHO_FOR_2D,
		x * g.window().x_highres(),
		y * g.window().y_highres(),
		w * g.window().x_highres(),
		h * g.window().y_highres()
	);

	glv::graphicsHolder().set(g);
	glv::graphicsHolder().size(w, h);

	g.pushMatrix();
	glv.drawWidgets(w, h, dsec);
	g.popMatrix();

	// g.uniformColor(1, 0, 0);
	// glv::frame(10, 10, 50, 50);
	// // glv::rectangle(10, 10, 50, 50);
}

void al_draw_glv(glv::GLV& glv, al::Graphics& g, double dsec) {
	al_draw_glv(glv, g, 0, 0, g.window().width(), g.window().height(), dsec);
}