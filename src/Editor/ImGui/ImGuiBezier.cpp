    // implementation
#include "ImGuiBezier.hpp"

    static inline
    double ease( int easetype, double t )
    {
        using namespace std;

        const double d = 1.f;
        const double pi = 3.1415926535897932384626433832795;
        const double pi2 = 3.1415926535897932384626433832795 / 2;

        double p = t/d;

        switch( easetype )
        {
            // Modeled after the line y = x
            default:
            case Tween::TYPE::LINEAR: {
                return p;
            }

            // Modeled after the parabola y = x^2
            case Tween::TYPE::QUADIN: {
                return p * p;
            }

            // Modeled after the parabola y = -x^2 + 2x
            case Tween::TYPE::QUADOUT: {
                return -(p * (p - 2));
            }

            // Modeled after the piecewise quadratic
            // y = (1/2)((2x)^2)             ; [0, 0.5)
            // y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
            case Tween::TYPE::QUADINOUT: {
                if(p < 0.5) {
                    return 2 * p * p;
                }
                else {
                    return (-2 * p * p) + (4 * p) - 1;
                }
            }

            // Modeled after the cubic y = x^3
            case Tween::TYPE::CUBICIN: {
                return p * p * p;
            }

            // Modeled after the cubic y = (x - 1)^3 + 1
            case Tween::TYPE::CUBICOUT: {
                double f = (p - 1);
                return f * f * f + 1;
            }

            // Modeled after the piecewise cubic
            // y = (1/2)((2x)^3)       ; [0, 0.5)
            // y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
            case Tween::TYPE::CUBICINOUT: {
                if(p < 0.5) {
                    return 4 * p * p * p;
                }
                else {
                    double f = ((2 * p) - 2);
                    return 0.5 * f * f * f + 1;
                }
            }

            // Modeled after the quartic x^4
            case Tween::TYPE::QUARTIN: {
                return p * p * p * p;
            }

            // Modeled after the quartic y = 1 - (x - 1)^4
            case Tween::TYPE::QUARTOUT: {
                double f = (p - 1);
                return f * f * f * (1 - p) + 1;
            }

            // Modeled after the piecewise quartic
            // y = (1/2)((2x)^4)        ; [0, 0.5)
            // y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
            case Tween::TYPE::QUARTINOUT:  {
                if(p < 0.5) {
                    return 8 * p * p * p * p;
                }
                else {
                    double f = (p - 1);
                    return -8 * f * f * f * f + 1;
                }
            }

            // Modeled after the quintic y = x^5
            case Tween::TYPE::QUINTIN:  {
                return p * p * p * p * p;
            }

            // Modeled after the quintic y = (x - 1)^5 + 1
            case Tween::TYPE::QUINTOUT:  {
                double f = (p - 1);
                return f * f * f * f * f + 1;
            }

            // Modeled after the piecewise quintic
            // y = (1/2)((2x)^5)       ; [0, 0.5)
            // y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
            case Tween::TYPE::QUINTINOUT:  {
                if(p < 0.5) {
                    return 16 * p * p * p * p * p;
                }
                else {
                    double f = ((2 * p) - 2);
                    return  0.5 * f * f * f * f * f + 1;
                }
            }

            // Modeled after quarter-cycle of sine wave
            case Tween::TYPE::SINEIN: {
                return sin((p - 1) * pi2) + 1;
            }

            // Modeled after quarter-cycle of sine wave (different phase)
            case Tween::TYPE::SINEOUT: {
                return sin(p * pi2);
            }

            // Modeled after half sine wave
            case Tween::TYPE::SINEINOUT: {
                return 0.5 * (1 - cos(p * pi));
            }

            // Modeled after shifted quadrant IV of unit circle
            case Tween::TYPE::CIRCIN: {
                return 1 - sqrt(1 - (p * p));
            }

            // Modeled after shifted quadrant II of unit circle
            case Tween::TYPE::CIRCOUT: {
                return sqrt((2 - p) * p);
            }

            // Modeled after the piecewise circular function
            // y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
            // y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
            case Tween::TYPE::CIRCINOUT: {
                if(p < 0.5) {
                    return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
                }
                else {
                    return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
                }
            }

            // Modeled after the exponential function y = 2^(10(x - 1))
            case Tween::TYPE::EXPOIN: {
                return (p == 0.0) ? p : pow(2, 10 * (p - 1));
            }

            // Modeled after the exponential function y = -2^(-10x) + 1
            case Tween::TYPE::EXPOOUT: {
                return (p == 1.0) ? p : 1 - pow(2, -10 * p);
            }

            // Modeled after the piecewise exponential
            // y = (1/2)2^(10(2x - 1))         ; [0,0.5)
            // y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
            case Tween::TYPE::EXPOINOUT: {
                if(p == 0.0 || p == 1.0) return p;
                
                if(p < 0.5) {
                    return 0.5 * pow(2, (20 * p) - 10);
                }
                else {
                    return -0.5 * pow(2, (-20 * p) + 10) + 1;
                }
            }

            // Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
            case Tween::TYPE::ELASTICIN: {
                return sin(13 * pi2 * p) * pow(2, 10 * (p - 1));
            }

            // Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
            case Tween::TYPE::ELASTICOUT: {
                return sin(-13 * pi2 * (p + 1)) * pow(2, -10 * p) + 1;
            }

            // Modeled after the piecewise exponentially-damped sine wave:
            // y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
            // y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
            case Tween::TYPE::ELASTICINOUT: {
                if(p < 0.5) {
                    return 0.5 * sin(13 * pi2 * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
                }
                else {
                    return 0.5 * (sin(-13 * pi2 * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
                }
            }

            // Modeled (originally) after the overshooting cubic y = x^3-x*sin(x*pi)
            case Tween::TYPE::BACKIN: { /*
                return p * p * p - p * sin(p * pi); */
                double s = 1.70158f;
                return p * p * ((s + 1) * p - s);
            }

            // Modeled (originally) after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
            case Tween::TYPE::BACKOUT: { /*
                double f = (1 - p);
                return 1 - (f * f * f - f * sin(f * pi)); */
                double s = 1.70158f;
                return --p, 1.f * (p*p*((s+1)*p + s) + 1);
            }

            // Modeled (originally) after the piecewise overshooting cubic function:
            // y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
            // y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
            case Tween::TYPE::BACKINOUT: { /*
                if(p < 0.5) {
                    double f = 2 * p;
                    return 0.5 * (f * f * f - f * sin(f * pi));
                }
                else {
                    double f = (1 - (2*p - 1));
                    return 0.5 * (1 - (f * f * f - f * sin(f * pi))) + 0.5;
                } */
                double s = 1.70158f * 1.525f;
                if (p < 0.5) {
                    return p *= 2, 0.5 * p * p * (p*s+p-s);
                }
                else {
                    return p = p * 2 - 2, 0.5 * (2 + p*p*(p*s+p+s));
                }
            }

#           define tween$bounceout(p) ( \
                (p) < 4/11.0 ? (121 * (p) * (p))/16.0 : \
                (p) < 8/11.0 ? (363/40.0 * (p) * (p)) - (99/10.0 * (p)) + 17/5.0 : \
                (p) < 9/10.0 ? (4356/361.0 * (p) * (p)) - (35442/1805.0 * (p)) + 16061/1805.0 \
                           : (54/5.0 * (p) * (p)) - (513/25.0 * (p)) + 268/25.0 )

            case Tween::TYPE::BOUNCEIN: {
                return 1 - tween$bounceout(1 - p);
            }

            case Tween::TYPE::BOUNCEOUT: {
                return tween$bounceout(p);
            }

            case Tween::TYPE::BOUNCEINOUT: {
                if(p < 0.5) {
                    return 0.5 * (1 - tween$bounceout(1 - p * 2));
                }
                else {
                    return 0.5 * tween$bounceout((p * 2 - 1)) + 0.5;
                }
            }

#           undef tween$bounceout

            case Tween::TYPE::SINESQUARE: {
                double A = sin((p)*pi2);
                return A*A;
            }

            case Tween::TYPE::EXPONENTIAL: {
                return 1/(1+exp(6-12*(p)));                
            }

            case Tween::TYPE::SCHUBRING1: {
                return 2*(p+(0.5f-p)*abs(0.5f-p))-0.5f;                
            }

            case Tween::TYPE::SCHUBRING2: {
                double p1pass= 2*(p+(0.5f-p)*abs(0.5f-p))-0.5f;
                double p2pass= 2*(p1pass+(0.5f-p1pass)*abs(0.5f-p1pass))-0.5f;
                double pAvg=(p1pass+p2pass)/2;
                return pAvg;
            }

            case Tween::TYPE::SCHUBRING3: {
                double p1pass= 2*(p+(0.5f-p)*abs(0.5f-p))-0.5f;
                double p2pass= 2*(p1pass+(0.5f-p1pass)*abs(0.5f-p1pass))-0.5f;
                return p2pass;
            }

            case Tween::TYPE::SWING: {
                return ((-cos(pi * p) * 0.5) + 0.5);
            }

            case Tween::TYPE::SINPI2: {
                return sin(p * pi2);
            }
        }
    }   

namespace ImGui
{
    // [src] http://iquilezles.org/www/articles/minispline/minispline.htm
    // key format (for dim == 1) is (t0,x0,t1,x1 ...)
    // key format (for dim == 2) is (t0,x0,y0,t1,x1,y1 ...)
    // key format (for dim == 3) is (t0,x0,y0,z0,t1,x1,y1,z1 ...)
    void spline( const float *key, int num, int dim, float t, float *v ) {
        static signed char coefs[16] = {
            -1, 2,-1, 0,
             3,-5, 0, 2,
            -3, 4, 1, 0,
             1,-1, 0, 0 };

        const int size = dim + 1;

        // find key
        int k = 0; while( key[k*size] < t ) k++;

        // interpolant
        const float h = (t-key[(k-1)*size])/(key[k*size]-key[(k-1)*size]);

        // init result
        for( int i=0; i < dim; i++ ) v[i] = 0.0f;

        // add basis functions
        for( int i=0; i<4; i++ )
        {
            int kn = k+i-2; if( kn<0 ) kn=0; else if( kn>(num-1) ) kn=num-1;

            const signed char *co = coefs + 4*i;

            const float b  = 0.5f*(((co[0]*h + co[1])*h + co[2])*h + co[3]);

            for( int j=0; j < dim; j++ ) v[j] += b * key[kn*size+j+1];
        }
    }

    float CurveValueSmooth(float p, int maxpoints, const ImVec2 *points){
        if (maxpoints < 2 || points == 0)
            return 0;
        if (p < 0) return points[0].y;

        float *input = new float [ maxpoints * 2 ];
        float output[4];

        for( int i = 0; i < maxpoints; ++i ) {
            input[ i * 2 + 0 ] = points[i].x;
            input[ i * 2 + 1 ] = points[i].y;
        }

        spline( input, maxpoints, 1, p, output );
        
        delete [] input;
        return output[0];
    }

    float CurveValue(float p, int maxpoints, const ImVec2 *points) {
        if (maxpoints < 2 || points == 0)
            return 0;
        if (p < 0) return points[0].y;

        int left = 0;
        while (left < maxpoints && points[left].x < p && points[left].x != -1) left++;
        if (left) left--;

        if (left == maxpoints-1)
            return points[maxpoints - 1].y;

        float d = (p - points[left].x) / (points[left + 1].x - points[left].x);

        return points[left].y + (points[left + 1].y - points[left].y) * d;
    }

    int Curve(const char *label, const ImVec2& size, const int maxpoints, ImVec2 *points) {
        int modified = 0;
        int i;
        if (maxpoints < 2 || points == 0)
            return 0;

        if (points[0].x < 0)
        {
            points[0].x = 0;
            points[0].y = 0;
            points[1].x = 1;
            points[1].y = 1;
            points[2].x = -1;
        }
        
        ImGuiWindow* window = GetCurrentWindow();
        //ImGuiState& g = *GImGui;
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiID id = window->GetID(label);
        if (window->SkipItems)
            return 0;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ItemSize(bb);
        if (!ItemAdd(bb, NULL))
            return 0;

        const bool hovered = IsHovered(bb, id);

        int max = 0;
        while (max < maxpoints && points[max].x >= 0) max++;

        int kill = 0;
        do
        {
            if (kill)
            {
                modified = 1;
                for (i = kill + 1; i < max; i++)
                {
                    points[i - 1] = points[i];
                }
                max--;
                points[max].x = -1;
                kill = 0;
            }

            for (i = 1; i < max - 1; i++)
            {
                if (abs(points[i].x - points[i - 1].x) < 1 / 128.0)
                {
                    kill = i;
                }
            }
        }
        while (kill);


        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, style.FrameRounding);

        float ht = bb.Max.y - bb.Min.y;
        float wd = bb.Max.x - bb.Min.x;

        if (hovered)
        {
            SetHoveredID(id);
            if (ImGui::GetIO().MouseDown[0])
            {
                modified = 1;
                ImVec2 pos = (ImGui::GetIO().MousePos - bb.Min) / (bb.Max - bb.Min);
                pos.y = 1 - pos.y;              

                int left = 0;
                while (left < max && points[left].x < pos.x) left++;
                if (left) left--;

                ImVec2 p = points[left] - pos;
                float p1d = sqrt(p.x*p.x + p.y*p.y);
                p = points[left+1] - pos;
                float p2d = sqrt(p.x*p.x + p.y*p.y);
                int sel = -1;
                if (p1d < (1 / 16.0)) sel = left;
                if (p2d < (1 / 16.0)) sel = left + 1;

                if (sel != -1)
                {
                    points[sel] = pos;
                }
                else
                {
                    if (max < maxpoints)
                    {
                        max++;
                        for (i = max; i > left; i--)
                        {
                            points[i] = points[i - 1];
                        }
                        points[left + 1] = pos;
                    }
                    if (max < maxpoints)
                        points[max].x = -1;
                }

                // snap first/last to min/max
                if( points[0].x < points[max - 1].x ) {
                    points[0].x= 0;
                    points[max - 1].x = 1;
                } else {
                    points[0].x= 1;
                    points[max - 1].x = 0;
                }
            }
        }

        // bg grid
        window->DrawList->AddLine(
            ImVec2(bb.Min.x, bb.Min.y + ht / 2),
            ImVec2(bb.Max.x, bb.Min.y + ht / 2),
            GetColorU32(ImGuiCol_TextDisabled), 3);

        window->DrawList->AddLine(
            ImVec2(bb.Min.x, bb.Min.y + ht / 4),
            ImVec2(bb.Max.x, bb.Min.y + ht / 4),
            GetColorU32(ImGuiCol_TextDisabled));

        window->DrawList->AddLine(
            ImVec2(bb.Min.x, bb.Min.y + ht / 4 * 3),
            ImVec2(bb.Max.x, bb.Min.y + ht / 4 * 3),
            GetColorU32(ImGuiCol_TextDisabled));

        for (i = 0; i < 9; i++)
        {
            window->DrawList->AddLine(
                ImVec2(bb.Min.x + (wd / 10) * (i + 1), bb.Min.y),
                ImVec2(bb.Min.x + (wd / 10) * (i + 1), bb.Max.y),
                GetColorU32(ImGuiCol_TextDisabled));
        }   

        // smooth curve
        enum { smoothness = 256 }; // the higher the smoother
        for( i = 0; i <= (smoothness-1); ++i ) {
            float px = (i+0) / float(smoothness);
            float qx = (i+1) / float(smoothness);
            float py = 1 - CurveValueSmooth(px, maxpoints, points);
            float qy = 1 - CurveValueSmooth(qx, maxpoints, points);
            ImVec2 p( px * (bb.Max.x - bb.Min.x) + bb.Min.x, py * (bb.Max.y - bb.Min.y) + bb.Min.y);
            ImVec2 q( qx * (bb.Max.x - bb.Min.x) + bb.Min.x, qy * (bb.Max.y - bb.Min.y) + bb.Min.y);
            window->DrawList->AddLine(p, q, GetColorU32(ImGuiCol_PlotLines));
        } 

        // lines
        for (i = 1; i < max; i++)
        {
            ImVec2 a = points[i - 1];
            ImVec2 b = points[i];
            a.y = 1 - a.y;
            b.y = 1 - b.y;
            a = a * (bb.Max - bb.Min) + bb.Min;
            b = b * (bb.Max - bb.Min) + bb.Min;
            window->DrawList->AddLine(a, b, GetColorU32(ImGuiCol_PlotLinesHovered));
        }

        if (hovered)
        {
            // control points
            for (i = 0; i < max; i++)
            {
                ImVec2 p = points[i];
                p.y = 1 - p.y;
                p = p * (bb.Max - bb.Min) + bb.Min;
                ImVec2 a = p - ImVec2(2, 2);
                ImVec2 b = p + ImVec2(2, 2);
                window->DrawList->AddRect(a, b, GetColorU32(ImGuiCol_PlotLinesHovered));
            }
        }

        // buttons; @todo: mirror, smooth, tessellate
        if( ImGui::Button("Flip") ) {
            for( i = 0; i < max; ++i) { 
                points[i].y = 1 - points[i].y;
            }
        }
        ImGui::SameLine();

        // curve selector
        const char* items[] = { 
            "Custom", 

            "Linear",
            "Quad in",
            "Quad out",
            "Quad in  out",
            "Cubic in",
            "Cubic out",
            "Cubic in  out",
            "Quart in",
            "Quart out",
            "Quart in  out",
            "Quint in",
            "Quint out",
            "Quint in  out",
            "Sine in",
            "Sine out",
            "Sine in  out",
            "Expo in",
            "Expo out",
            "Expo in  out",
            "Circ in",
            "Circ out",
            "Circ in  out",
            "Elastic in",
            "Elastic out",
            "Elastic in  out",
            "Back in",
            "Back out",
            "Back in  out",
            "Bounce in",
            "Bounce out",
            "Bounce in out",

            "Sine square",
            "Exponential",

            "Schubring1",
            "Schubring2",
            "Schubring3",

            "SinPi2",
            "Swing"
        };
        static int item = 0;
        if( modified ) {
            item = 0;
        }
        if( ImGui::Combo("Ease type", &item, items, IM_ARRAYSIZE(items)) ) {
            max = maxpoints;
            if( item > 0 ) {
                for( i = 0; i < max; ++i) { 
                    points[i].x = i / float(max-1); 
                    points[i].y = float( ease( item - 1, points[i].x ) );
                }               
            }
        }

        char buf[128];
        const char *str = label;

        if( hovered ) {
            ImVec2 pos = (ImGui::GetIO().MousePos - bb.Min) / (bb.Max - bb.Min);
            pos.y = 1 - pos.y;              

            sprintf(buf, "%s (%f,%f)", label, pos.x, pos.y );
            str = buf;
        }       
       
        //RenderTextClipped(ImVec2(bb.Min.x, bb.Min.y + style.FramePadding.y), bb.Max, str, NULL, NULL, ImGuiAlign_Center);

        return modified;
    }

}
