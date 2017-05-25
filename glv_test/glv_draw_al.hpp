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
	void set(al::Graphics& g) {
		mGraphics = &g;
	}
	al::Graphics& get() {
		return *mGraphics;
	}
};

GraphicsHolder& graphicsHolder() {
	static GraphicsHolder graphicsHolder;
	return graphicsHolder;
}

/// Returns closest pixel coordinate
int pix(float v) {
    return v >= 0 ? (int)(v+0.5f) : (int)(v-0.5f);
}
/// Returns center of closest pixel coordinate
float pixc(float v) {
    return pix(v) + 0.5f;
}

void rectangle(float l, float t, float r, float b) {
	static al::VAOMesh mesh;

	float x = l;
    float y = t;
	float w = r - l;
	float h = b - t;
	mesh.reset();
	addRect(mesh, x, y, w, h);
	mesh.update();

    graphicsHolder().get().draw(mesh);
}

void frame(float l, float t, float r, float b) {
	static al::VAOMesh mesh;

	float x = l;
    float y = t;
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

    graphicsHolder().get().draw(mesh);
}

void line(float x0, float y0, float x1, float y1) {
	static al::VAOMesh mesh;

	mesh.reset();
	mesh.primitive(al::Mesh::LINES);
    mesh.vertex(x0, y0);
    mesh.vertex(x1, y1);
	mesh.update();

    graphicsHolder().get().draw(mesh);
}

void lines(GraphicsData& gd) {
	static al::EasyVAO vao;
	vao.primitive(GL_LINES);
	vao.updatePosition(gd.vertices3().data(), gd.vertices3().size());
	graphicsHolder().get().draw(vao);
}

void grid (
	float l, float t, float w, float h,
	float divx, float divy,
	bool incEnds=true
) {
	static al::VAOMesh mesh;

	mesh.reset();
	mesh.primitive(al::Mesh::LINES);
	double inc, r=l+w, b=t+h;

	if(divy > 0 && h>0){
		inc = (double)h/(double)divy;
		double i = incEnds ? t-0.0001 : t-0.0001+inc;
		double e = incEnds ? b : b-inc;
		for(; i<e; i+=inc) {
            mesh.vertex(l, i);
            mesh.vertex(r, i);
		}
	}

	if(divx > 0 && w>0){
		inc = (double)w/(double)divx;
		double i = incEnds ? l-0.0001 : l-0.0001+inc;
		double e = incEnds ? r : r-inc;
		for(; i<e; i+=inc) {
            mesh.vertex(i, t);
            mesh.vertex(i, b);
		}
	}
	mesh.update();
    graphicsHolder().get().draw(mesh);
}

void pushMatrix() {
    graphicsHolder().get().pushMatrix();
}
void popMatrix() {
    graphicsHolder().get().popMatrix();
}

void loadIdentity() {
	graphicsHolder().get().loadIdentity();
}

void translate(float x, float y) {
	graphicsHolder().get().translate(x, y);
}

void rotate(float angleX, float angleY, float angleZ) {
	graphicsHolder().get().rotate(angleX, angleY, angleZ);
}

void scale(float scaleX, float scaleY) {
    graphicsHolder().get().scale(scaleX, scaleY);
}

void scissorTest(bool doScissor) {
	graphicsHolder().get().scissorTest(doScissor);
}

void scissor(int x, int y, int w, int h) {
	graphicsHolder().get().scissor(x, y, w, h);
}

float windowHighresFactorX() {
	return graphicsHolder().get().window().x_highres();
}

float windowHighresFactorY() {
	return graphicsHolder().get().window().y_highres();
}

void color(float r, float g, float b, float a) {
	graphicsHolder().get().uniformColor(r, g, b, a);
}

void color(Color const& c) {
	color(c.r, c.g, c.b, c.a);
}

void text(
	const char * s,
	float l=0, float t=0,
	unsigned fontSize=8, float lineSpacing=1.5, unsigned tabSpaces=4
) {
	Font f;
	f.size(fontSize);
	f.lineSpacing(lineSpacing);
	f.tabSpaces(tabSpaces);
	f.render(s, l, t, 0);
}

void drawContext (float tx, float ty, View * v, float& cx, float& cy, View *& c) {
	cx += tx; cy += ty; // update absolute coordinates of drawing context

    // clear model matrix (assumed set already)
    // it was loadIdentity() before but
    // pop then push preserves previous matrix (the one before pushing)
    popMatrix();
    pushMatrix();

	// pix: round position to nearest pixel coordinates
	translate(pix(cx), pix(cy));
	c = v;
}

void computeCrop(std::vector<Rect>& cr, int lvl, space_t ax, space_t ay, View * v) {
	if (v->enabled(CropChildren)) {
		cr[lvl].set(ax, ay, v->w, v->h);	// set absolute rect
		// get intersection with myself and previous level
		if (lvl>0) {
			Rect r = cr[lvl];
			// r.resizeEdgesBy(-1);	// shrink area to save the borders
			r.intersection(cr[lvl-1], cr[lvl]);
		}
	}
	// if no child cropping, then inherit previous level's crop rect
	else{ cr[lvl] = cr[lvl-1]; }
}

// Views are drawn depth-first from leftmost to rightmost sibling
void GLV::drawWidgets(unsigned int ww, unsigned int wh, double dsec) {
	

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
	// struct AnimateViews : public TraversalAction{
	// 	AnimateViews(double dt_): dt(dt_){}
	// 	virtual bool operator()(View * v, int depth){
	// 		if(v->enabled(Animate)) v->onAnimate(dt);
	// 		return true;
	// 	}
	// 	double dt;
	// } animateViews(dsec);
	// traverseDepth(animateViews);

	graphicsData().reset();
	doDraw(*this);

	//scissorTest(true);

	while(true){

		cv->onDataModelSync(); // update state based on attached model variables
		cv->rectifyGeometry();

        pushMatrix();
		// find the next view to draw

		// go to child node if exists and I'm drawable
		if(cv->child && cv->visible()) {
			drawContext(cv->child->l, cv->child->t, cv->child, cx, cy, cv);
			computeCrop(cropRects, ++lvl, cx, cy, cv);
		}
		else if(cv->sibling) { // go to sibling node if exists
			drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
			computeCrop(cropRects, lvl, cx, cy, cv);
		}
		else { // retrace upwards until a parent's sibling is found
			while(cv != root && cv->sibling == 0) {
				drawContext(-cv->l, -cv->t, cv->parent, cx, cy, cv);
				lvl--;
			}

			if(cv->sibling){
				drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
				computeCrop(cropRects, lvl, cx, cy, cv);
			}
			else {
                break; // break the loop when the traversal returns to the root
            }
		}
		
		// draw current view
		if(cv->visible()){
			Rect r = cropRects[lvl-1]; // cropping region comes from parent context

            
            /*
			if(cv->enabled(CropSelf)) { // crop my own draw?
                r.intersection(Rect(cx, cy, cv->w, cv->h), r);
            }
            */

			if(r.h<=0.f || r.w <= 0.f) { // bypass if drawing area outside of crop region
                // std::cout << "bypassing" << std::endl;
                continue;
            }

			// int sx = pix(r.l);
			// int sy = wh - (pix(r.t) + pix(r.h)) + 0.99; // scissor coord has origin at bottom left
			// int sw = pix(r.w);
			// int sh = r.h + 0.5;
			// if(sy < 0) sy=0;

			// glScissor takes size in framebuffer dimension
			//scissor(
			//	sx * windowHighresFactorX(),
			//	sy * windowHighresFactorY(),
			//	sw * windowHighresFactorX(),
			//	sh * windowHighresFactorY()
			//);

			graphicsData().reset();
			cv->doDraw(*this);
		}

        popMatrix();
	}

	//scissorTest(false);
}

void View::doDraw(GLV& glv){
	if(enabled(DrawBack)){
		color(colors().back);
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
		color(colors().border);
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
		// lineWidth(1); // disabled in >gl3
		color(colors().border);
		grid(0, 0, w, h, sizeX(), sizeY(), false);
	}
}

void Widget::drawSelectionBox(){
	if(enabled(Focused) && enabled(DrawSelectionBox) && size()>1){
		// lineWidth(2); // disabled in >gl3
		color(colors().border);
		frame(sx*dx(), sy*dy(), (sx+1)*dx(), (sy+1)*dy());
	}
}

void Widget::onDraw(GLV& g){
	drawSelectionBox();
	drawGrid();
	g.graphicsData().reset();
}

void Sliders::onDraw(GLV& glv) {
	Widget::onDraw(glv);

	float x=paddingX(), xd=dx(), yd=dy();

	// TODO: fix padding in orientation direction

	if(vertOri()){
		for(int i=0; i<sizeX(); ++i){
		
			float y=paddingY();
		
			for(int j=0; j<sizeY(); ++j){
				int ind = index(i,j);
				auto const& cf = colors().fore;
				if(isSelected(i,j)) color(cf.r, cf.g, cf.b, cf.a);
				else                color(cf.r, cf.g, cf.b, cf.a * 0.5);

				float v01 = to01(getValue(ind));
				//float y0 = to01(0)*(yd - paddingY()*2);
				float y0 = to01(0)*yd;
				//rect(x + x0, y, f*xd+x, y+yd-padding());
				
				rectangle(x, y + (yd-v01*(yd-paddingY()*2)), x+xd-paddingX()*2, y + (yd-y0));

				// if zero line showing
				if(max()>0 && min()<0){
					color(colors().border);
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
				if(isSelected(i,j)) color(cf.r, cf.g, cf.b, cf.a);
				else                color(cf.r, cf.g, cf.b, cf.a * 0.5);

				float v01 = to01(getValue(ind));
				float x0 = to01(0)*xd;
				rectangle(x + x0, y, v01*(xd-paddingX()*2)+x, y+yd-paddingY()*2);

				// if zero line showing
				if(max()>0 && min()<0){
					color(colors().border);
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
	//static bool print_once = [](){ std::cout << "Font::render" << std::endl; return true; }();

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
	
	// draw::paint(draw::Lines, gd);
	lines(gd);
}

void SliderRange::onDraw(GLV& g) {
	//static bool print_once = [](){ std::cout << "SliderRange::onDraw" << std::endl; return true; }();

}

void Slider2D::onDraw(GLV& g) {
	//static bool print_once = [](){ std::cout << "Slider2D::onDraw" << std::endl; return true; }();

}

void Label::onDraw(GLV& g){
	// static bool print_once = [](){ std::cout << "Label::onDraw" << std::endl; return true; }();
	// lineWidth(stroke()); 
	color(colors().text);
	if(mVertical){
        translate(0,h);
        rotate(0,0,-90);
    }
	font().render(
		g.graphicsData(),
		data().toString().c_str(),
		paddingX(),
		paddingY()
	);
	// scale(mSize, mSize);
	// text(value().c_str());
}



void NumberDialers::fitExtent() {
	//static bool print_once = [](){ std::cout << "NumberDialers::fitExtent" << std::endl; return true; }();

}


void NumberDialers::onDraw(GLV& g) {
	//static bool print_once = [](){ std::cout << "NumberDialers::onDraw" << std::endl; return true; }();

}

void DropDown::onDraw(GLV& g){
	//static bool print_once = [](){ std::cout << "DropDown::onDraw" << std::endl; return true; }();
}

ListView& ListView::fitExtent(){
	//static bool print_once = [](){ std::cout << "ListView::fitExtent" << std::endl; return true; }();
    return *this;
}


void ListView::onDraw(GLV& g){
	//static bool print_once = [](){ std::cout << "ListView::onDraw" << std::endl; return true; }();
}

void TextView::onDraw(GLV& g){
	//static bool print_once = [](){ std::cout << "TextView::onDraw" << std::endl; return true; }();
}

void Table::onDraw(GLV& g){

}

void Scroll::onDraw(GLV& g){

}

void Divider::onDraw(GLV& g){
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


#endif
// -----------------------------------------------------------------------------

} // namespace glv

void al_draw_glv(
	glv::GLV& glv, al::Graphics& g,
	unsigned x, unsigned y, unsigned w, unsigned h, double dsec
) {
	// g.lighting(false);
	g.depthTesting(false);
	g.blending(true);
	g.blendModeTrans();
	g.uniformColorMix(1);
    g.textureMix(0, 0, 0, 0);
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
	//glv::graphicsHolder().size(w, h);

	g.pushMatrix();
    g.loadIdentity();
    g.translate(0, g.window().height()); // move to top-right
    g.scale(1, -1); // flip y
	glv.drawWidgets(w, h, dsec);
	g.popMatrix();
}

void al_draw_glv(glv::GLV& glv, al::Graphics& g, double dsec) {
	al_draw_glv(glv, g, 0, 0, g.window().width(), g.window().height(), dsec);
}