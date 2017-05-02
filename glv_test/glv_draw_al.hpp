#pragma once

#include "al/core.hpp"

#include "al/glv/glv_core.h" // GLV, View
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
	bool incEnds=true)
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
	unsigned fontSize=8, float lineSpacing=1.5, unsigned tabSpaces=4)
{
	Font f;
	f.size(fontSize);
	f.lineSpacing(lineSpacing);
	f.tabSpaces(tabSpaces);
	f.render(s, l, t, 0);
}

void drawContext (
	float tx, float ty, View * v, float& cx, float& cy, View *& c)
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
	std::vector<Rect>& cr, int lvl, space_t ax, space_t ay, View * v)
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
		// float borderWidth = 1.0;

		// double border thickness if focused
		// if(enabled(Focused) && enabled(FocusHighlight)){
		// 	borderWidth *= 2;
		// }

		// g.lineWidth(borderWidth); // disabled in >gl3
		auto const& cb = colors().border;
		g.uniformColor(cb.r, cb.g, cb.b, cb.a);
		// 1.0 and 0.5: it's hack anyway
		frame(1.0, 1.0, pix(w)-0.5, pix(h)-0.5);
		// frame(1.5, 1.5, pix(w)-1.0, pix(h)-1.0);
		// const float ds = 0.5; // OpenGL suggests 0.375, but smears with AA enabled
		// frame(ds, ds, pix(w)-ds, pix(h)-ds);
		// frame(0,0, pix(w)-ds, pix(h)-ds); // hack to give bevelled look

	}
}

void Widget::drawGrid(){
	if(enabled(DrawGrid) && size()>1){
		auto& g = graphicsHolder().get();
		// g.lineWidth(1); // disabled in >gl3
		auto const& cb = colors().border;
		g.uniformColor(cb.r, cb.g, cb.b, cb.a);
		grid(0, 0, w, h, sizeX(), sizeY(), false);
	}
}

void Widget::drawSelectionBox(){
	if(enabled(Focused) && enabled(DrawSelectionBox) && size()>1){
		auto& g = graphicsHolder().get();
		g.lineWidth(2);
		auto const& cb = colors().border;
		g.uniformColor(cb.r, cb.g, cb.b, cb.a);
		frame(sx*dx(), sy*dy(), (sx+1)*dx(), (sy+1)*dy());
	}
}

void Widget::onDraw(GLV& g){
	drawSelectionBox();
	drawGrid();
	// g.graphicsData().reset();
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
				auto const& cf = colors().fore;
				if(isSelected(i,j)) g.uniformColor(cf.r, cf.g, cf.b, cf.a);
				else                g.uniformColor(cf.r, cf.g, cf.b, cf.a * 0.5);

				float v01 = to01(getValue(ind));
				//float y0 = to01(0)*(yd - paddingY()*2);
				float y0 = to01(0)*yd;
				//rect(x + x0, y, f*xd+x, y+yd-padding());
				
				rectangle(x, y + (yd-v01*(yd-paddingY()*2)), x+xd-paddingX()*2, y + (yd-y0));

				// if zero line showing
				if(max()>0 && min()<0){
					auto const& cb = colors().border;
					g.uniformColor(cb.r, cb.g, cb.b);
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
				auto const& cf = colors().fore;
				if(isSelected(i,j)) g.uniformColor(cf.r, cf.g, cf.b, cf.a);
				else                g.uniformColor(cf.r, cf.g, cf.b, cf.a * 0.5);

				float v01 = to01(getValue(ind));
				float x0 = to01(0)*xd;
				rectangle(x + x0, y, v01*(xd-paddingX()*2)+x, y+yd-paddingY()*2);

				// if zero line showing
				if(max()>0 && min()<0){
					auto const& cb = colors().border;
					g.uniformColor(cb.r, cb.g, cb.b);
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

void Label::onDraw(GLV& g){
	// lineWidth(stroke());
	// color(colors().text);
	// if(mVertical){ translate(0,h); rotate(0,0,-90); }
	// font().render(
	// 	g.graphicsData(),
	// 	data().toString().c_str(),
	// 	paddingX(),
	// 	paddingY()
	// );
	//scale(mSize, mSize);
	//text(value().c_str());
}



void NumberDialers::fitExtent(){

}


void NumberDialers::onDraw(GLV& g){

}

void DropDown::onDraw(GLV& g){
}

ListView& ListView::fitExtent(){
    return ListView {};
}


void ListView::onDraw(GLV& g){
}

void TextView::onDraw(GLV& g){

}

// -----------------------------------------------------------------------------
#ifdef DONT_COMMENT_OUT

void Buttons::onDraw(GLV& g){
	Widget::onDraw(g);

	using namespace glv::draw;

	float xd = dx();
	float yd = dy();
	float padx = paddingX();
	float pady = paddingY();
	color(colors().fore);
	
	stroke(1);

	for (int i=0; i<sizeX(); ++i) {
		float x = xd*i + padx;
		for (int j=0; j<sizeY(); ++j) {
			float y = yd*j + pady;
			if (getValue(i,j)) {
				// on
				if(mSymOn ) mSymOn (x, y, x+xd-padx*2, y+yd-pady*2);
			}
			else {
				// off
				if(mSymOff) mSymOff(x, y, x+xd-padx*2, y+yd-pady*2);
			}
		}		
	}
	
}

void Slider2D::onDraw(GLV& g) {

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