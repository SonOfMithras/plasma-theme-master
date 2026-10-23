#!/usr/bin/env bash
# WhiteSur folder icons — dynamic color template for PTM.
# Thin wrapper: reads colors-final, finds the closest WhiteSur palette
# color to the target, then delegates to whitesur-folders for the
# actual overlay work.
set -euo pipefail

CONFIG_DIR="$(cd "$(dirname "$0")" && pwd)"
COLOR_FILE="${1:-$CONFIG_DIR/colors-final}"
if [[ ! -f "$COLOR_FILE" ]]; then
  COLOR_FILE="${XDG_CACHE_HOME:-$HOME/.cache}/ptm/whitesur-icons/colors-final"
fi

[[ -f "$COLOR_FILE" ]] || exit 0

mapfile -t lines <"$COLOR_FILE"
TARGET="${lines[0]//[# ]/}"
[[ ${#TARGET} -ge 6 ]] || exit 0
TARGET="${TARGET:0:6}"
TR=$((16#${TARGET:0:2}))
TG=$((16#${TARGET:2:2}))
TB=$((16#${TARGET:4:2}))

MAPPING="$(printf '%s\n' "${lines[@]}" | sed '/^[[:space:]]*$/d' | tail -n1)"

# HSV closest-color, hue-family aware (same intent as papirus-icons),
# in a single awk pass — no bc, no extra subshells for the override step.
closest=$(awk -v r="$TR" -v g="$TG" -v b="$TB" -v m="$MAPPING" '
function hex2dec(h,   i,c,v,digits) {
	digits = "0123456789abcdef"
	h = tolower(h)
	v = 0
	for (i=1; i<=length(h); i++) v = v*16 + (index(digits, substr(h,i,1)) - 1)
	return v
}
function rgb2hsv(r,g,b,  mx,mn,d,h,s,v,t) {
	r/=255; g/=255; b/=255
	mx=(r>g)?(r>b?r:b):(g>b?g:b); mn=(r<g)?(r<b?r:b):(g<b?g:b)
	v=mx; d=mx-mn
	if (d==0) { s=0; h=0 }
	else {
		s=d/mx
		if (mx==r) { t=(g-b)/d; if(t<0) t+=6; h=60*t }
		else if (mx==g) { h=60*(((b-r)/d)+2) }
		else { h=60*(((r-g)/d)+4) }
	}
	return h SUBSEP s SUBSEP v
}
function family(h,s) {
	if (s<0.1)          return "neutral"
	if (h>=340||h<20)   return "red"
	if (h>=20&&h<50)    return "warm"
	if (h>=50&&h<180)   return "green"
	if (h>=180&&h<260)  return "blue"
	if (h>=260&&h<310)  return "cool"
	if (h>=310&&h<340)  return "pink"
	return "neutral"
}
BEGIN {
	WH=10; WS=1; WV=0.3
	split(rgb2hsv(r,g,b), tgt, SUBSEP)
	th=tgt[1]+0; ts=tgt[2]+0; tv=tgt[3]+0
	tf = family(th, ts)   # target family — now saturation-aware, so a
	                       # near-gray target is never bucketed into a
	                       # noisy/unstable hue family (fixes an edge
	                       # case the original bc-based version had).

	n = split(m, arr)
	for (i=1; i<=n; i++) {
		split(arr[i], p, ":")
		cr=hex2dec(substr(p[2],1,2))
		cg=hex2dec(substr(p[2],3,2))
		cb=hex2dec(substr(p[2],5,2))
		split(rgb2hsv(cr,cg,cb), c, SUBSEP)
		ch=c[1]+0; cs=c[2]+0; cv=c[3]+0

		dh=th-ch; if(dh<0) dh=-dh
		if(dh>180) dh=360-dh; dh/=180
		ds=ts-cs; dv=tv-cv
		d = WH*(ts*cs)*dh*dh + WS*ds*ds + WV*dv*dv

		if (min=="" || d<min) { min=d; best=p[1] }
		if (family(ch,cs)==tf && (fmin=="" || d<fmin)) { fmin=d; fbest=p[1] }
	}
	# Prefer the nearest candidate that shares the targets hue family;
	# fall back to the plain nearest match when the target is neutral
	# or no candidate shares its family. Mirrors the original
	# override-only-when-chromatic-and-mismatched behavior.
	print (tf != "neutral" && fbest != "" ? fbest : best)
}')

# Invoked via bash so this keeps working even if a git checkout or zip
# download drops the executable bit on whitesur-folders.
exec bash "$CONFIG_DIR/whitesur-folders" "$closest"
