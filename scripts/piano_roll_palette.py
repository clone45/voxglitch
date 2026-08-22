#!/usr/bin/env python3
"""
Generates and verifies the Piano Roll's eight track colours.

Builds the palette in CIELCh against the editor's dark lanes, then measures the
worst-case separation between tracks with CIEDE2000 -- for normal vision and for
simulated deuteranopia and protanopia (Vienot, Brettel & Mollon 1999) -- both at
full opacity and dimmed, the way inactive tracks are drawn.

Prints a ready-to-paste C++ table for src/modules/PianoRoll/PianoRollPalette.hpp.

Run:  python3 scripts/piano_roll_palette.py
"""

import math, itertools

# ---------- colour space ----------
def srgb_to_lin(c):
    c/=255.0
    return c/12.92 if c<=0.04045 else ((c+0.055)/1.055)**2.4
def lin_to_srgb(c):
    v = c*12.92 if c<=0.0031308 else 1.055*(c**(1/2.4))-0.055
    return max(0,min(255,round(v*255)))
M_XYZ=[[0.4124564,0.3575761,0.1804375],[0.2126729,0.7151522,0.0721750],[0.0193339,0.1191920,0.9503041]]
WP=(0.95047,1.0,1.08883)
def rgb_to_xyz(rgb):
    r,g,b=[srgb_to_lin(c) for c in rgb]
    return tuple(sum(M[i]*v for M,v in zip([row[j] for row in M_XYZ],(r,g,b))) if False else
                 M_XYZ[i][0]*r+M_XYZ[i][1]*g+M_XYZ[i][2]*b for i in range(3))
def f(t): return t**(1/3) if t>216/24389 else (841/108)*t+4/29
def xyz_to_lab(xyz):
    x,y,z=[xyz[i]/WP[i] for i in range(3)]
    fx,fy,fz=f(x),f(y),f(z)
    return (116*fy-16, 500*(fx-fy), 200*(fy-fz))
def rgb_to_lab(rgb): return xyz_to_lab(rgb_to_xyz(rgb))
def hex_to_rgb(h): h=h.lstrip('#'); return tuple(int(h[i:i+2],16) for i in (0,2,4))
def rgb_to_hex(rgb): return '#%02X%02X%02X'%rgb

def lab_to_xyz(lab):
    L,a,b=lab; fy=(L+16)/116; fx=fy+a/500; fz=fy-b/200
    def fi(t): return t**3 if t**3>216/24389 else (108/841)*(t-4/29)
    return tuple(fi(v)*w for v,w in zip((fx,fy,fz),WP))
M_RGB=[[3.2404542,-1.5371385,-0.4985314],[-0.9692660,1.8760108,0.0415560],[0.0556434,-0.2040259,1.0572252]]
def xyz_to_rgb(xyz):
    x,y,z=xyz
    return tuple(lin_to_srgb(M_RGB[i][0]*x+M_RGB[i][1]*y+M_RGB[i][2]*z) for i in range(3))
def in_gamut(xyz):
    x,y,z=xyz
    for i in range(3):
        v=M_RGB[i][0]*x+M_RGB[i][1]*y+M_RGB[i][2]*z
        if v<-1e-4 or v>1+1e-4: return False
    return True
def lch_to_rgb_clipped(L,C,h):
    """Reduce chroma until in gamut."""
    while C>0:
        lab=(L, C*math.cos(math.radians(h)), C*math.sin(math.radians(h)))
        if in_gamut(lab_to_xyz(lab)): return xyz_to_rgb(lab_to_xyz(lab)), C
        C-=0.5
    return xyz_to_rgb(lab_to_xyz((L,0,0))), 0

# ---------- deltaE 2000 ----------
def de2000(lab1,lab2):
    L1,a1,b1=lab1; L2,a2,b2=lab2
    kL=kC=kH=1.0
    C1=math.hypot(a1,b1); C2=math.hypot(a2,b2); Cb=(C1+C2)/2
    G=0.5*(1-math.sqrt(Cb**7/(Cb**7+25**7))) if Cb>0 else 0.5
    a1p,a2p=(1+G)*a1,(1+G)*a2
    C1p,C2p=math.hypot(a1p,b1),math.hypot(a2p,b2)
    h1p=math.degrees(math.atan2(b1,a1p))%360 if (a1p or b1) else 0
    h2p=math.degrees(math.atan2(b2,a2p))%360 if (a2p or b2) else 0
    dLp=L2-L1; dCp=C2p-C1p
    if C1p*C2p==0: dhp=0
    elif abs(h2p-h1p)<=180: dhp=h2p-h1p
    elif h2p-h1p>180: dhp=h2p-h1p-360
    else: dhp=h2p-h1p+360
    dHp=2*math.sqrt(C1p*C2p)*math.sin(math.radians(dhp)/2)
    Lbp=(L1+L2)/2; Cbp=(C1p+C2p)/2
    if C1p*C2p==0: hbp=h1p+h2p
    elif abs(h1p-h2p)<=180: hbp=(h1p+h2p)/2
    elif h1p+h2p<360: hbp=(h1p+h2p+360)/2
    else: hbp=(h1p+h2p-360)/2
    T=1-0.17*math.cos(math.radians(hbp-30))+0.24*math.cos(math.radians(2*hbp))+ \
      0.32*math.cos(math.radians(3*hbp+6))-0.20*math.cos(math.radians(4*hbp-63))
    dTh=30*math.exp(-((hbp-275)/25)**2)
    Rc=2*math.sqrt(Cbp**7/(Cbp**7+25**7)) if Cbp>0 else 0
    Sl=1+(0.015*(Lbp-50)**2)/math.sqrt(20+(Lbp-50)**2)
    Sc=1+0.045*Cbp; Sh=1+0.015*Cbp*T
    Rt=-math.sin(math.radians(2*dTh))*Rc
    return math.sqrt((dLp/(kL*Sl))**2+(dCp/(kC*Sc))**2+(dHp/(kH*Sh))**2+
                     Rt*(dCp/(kC*Sc))*(dHp/(kH*Sh)))


# ---------- CVD: Vienot, Brettel & Mollon (1999), standard matrices ----------
def simulate(rgb, kind):
    if not kind: return rgb
    R,G,B=[srgb_to_lin(c) for c in rgb]
    L = 17.8824*R + 43.5161*G + 4.11935*B
    M = 3.45565*R + 27.1554*G + 3.86714*B
    S = 0.0299566*R + 0.184309*G + 1.46709*B
    if kind=='protan':  L = 2.02344*M - 2.52581*S
    elif kind=='deutan': M = 0.494207*L + 1.24827*S
    r =  0.080944447*L - 0.130504409*M + 0.116721066*S
    g = -0.010248534*L + 0.054019327*M - 0.113614708*S
    b = -0.000365297*L - 0.004121615*M + 0.693511405*S
    return tuple(lin_to_srgb(max(0.0,min(1.0,c))) for c in (r,g,b))

def blend(fg,bg,alpha):
    return tuple(round(fg[i]*alpha+bg[i]*(1-alpha)) for i in range(3))

import itertools
LANE_NAT=hex_to_rgb('#182A2A'); LANE_SHP=hex_to_rgb('#122222')
def min_sep(sw,kind=None):
    labs=[(t,rgb_to_lab(simulate(c,kind))) for t,c in sw]; worst=1e9; pair=None
    for (t1,l1),(t2,l2) in itertools.combinations(labs,2):
        if t1==t2: continue
        d=de2000(l1,l2)
        if d<worst: worst,pair=d,(t1,t2)
    return worst,pair

# Eight hues spread right around the wheel, with a slight lightness zig-zag so
# neighbouring tracks differ on two axes rather than one.   (name, hue, L*)
SPEC=[("Red",28,66),("Amber",68,76),("Yellow",98,70),("Green",140,72),
      ("Teal",185,68),("Azure",245,70),("Violet",290,64),("Magenta",330,70)]

def at(L,h,C): return lch_to_rgb_clipped(max(4,min(97,L)),C,h)[0]

rows=[]
for name,h,L in SPEC:
    fill,C = lch_to_rgb_clipped(L, 95, h)
    rows.append(dict(name=name, fill=fill, L=L, h=h, C=C,
        edge=at(L-18,h,80),
        sel=at(L+9,h,105),
        # Selection rim picked for CONTRAST against the fill: dark on the light
        # tracks, light on the dark ones, so it stays visible at both ends.
        sel_edge=at(L-34,h,60) if L>=70 else at(L+28,h,70)))

print("%-3s %-8s %-9s %-9s %-9s %-9s %5s"%("#","name","fill","edge","sel","selEdge","C*"))
for i,r in enumerate(rows):
    print("%-3d %-8s %-9s %-9s %-9s %-9s %5.1f"%(i+1,r['name'],rgb_to_hex(r['fill']),
        rgb_to_hex(r['edge']),rgb_to_hex(r['sel']),rgb_to_hex(r['sel_edge']),r['C']))

fills=[(i,r['fill']) for i,r in enumerate(rows)]
print("\nFULL opacity (active track):")
for kind,label in [(None,'normal'),('deutan','deutan'),('protan','protan')]:
    w,pr=min_sep(fills,kind)
    print(f"  {label:7s} min dE {w:5.2f}   worst: {rows[pr[0]]['name']}/{rows[pr[1]]['name']}")

print("\nDIMMED inactive tracks (both lane types):")
for a in (0.40,0.50,0.60):
    dim=[(i,blend(r['fill'],L,a)) for i,r in enumerate(rows) for L in (LANE_NAT,LANE_SHP)]
    v=[min_sep(dim,k)[0] for k in (None,'deutan','protan')]
    print(f"  a={a:.2f}  normal {v[0]:5.2f}  deutan {v[1]:5.2f}  protan {v[2]:5.2f}")

print("\n// Paste into src/modules/PianoRoll/PianoRollPalette.hpp")
for r in rows:
    def c(x): return "nvgRGB(0x%02X, 0x%02X, 0x%02X)"%x
    print("        { %s, %s,\n          %s, %s },  // %s"%(
        c(r['fill']),c(r['edge']),c(r['sel']),c(r['sel_edge']),r['name']))
