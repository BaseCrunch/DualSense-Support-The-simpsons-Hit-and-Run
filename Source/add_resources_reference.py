import struct, pathlib, sys

src = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2])

TOP = ("META.INI", """[Miscellaneous]\r\nInternalName=NexusDualSensePack\r\nTitle=Nexus DualSense\r\nDescription=Standalone Lucas Mod Launcher native DualSense support for The Simpsons: Hit & Run.\r\nSupportsEnglish=1\r\nSupportsDemo=0\r\nSupportsInternational=1\r\nSupportsBestSellerSeries=1\r\nRequiredLauncher=1.27.1\r\nPack=1\r\n\r\n[Author]\r\nName=Springfield Nexus\r\n""".encode('utf-8'))
SUB = ("NEXUSDUALSENSE\\META.INI", """[Miscellaneous]\r\nInternalName=NexusDualSense\r\nTitle=Nexus DualSense\r\nDescription=Native DualSense controller support using SDL3 and the Springfield Nexus SHAR input and haptics bridge.\r\nMod=1\r\nSetting=1\r\nSupportsEnglish=1\r\nSupportsDemo=0\r\nSupportsInternational=1\r\nSupportsBestSellerSeries=1\r\nRequiredLauncher=1.27.1\r\n\r\n[Author]\r\nName=Springfield Nexus\r\n""".encode('utf-8'))
resources=[TOP,SUB]

b=bytearray(src.read_bytes())
pe=struct.unpack_from('<I',b,0x3c)[0]
if b[pe:pe+4] != b'PE\0\0': raise SystemExit('not PE')
coff=pe+4
machine,nsects=struct.unpack_from('<HH',b,coff)
if machine != 0x14c: raise SystemExit('not x86')
szopt=struct.unpack_from('<H',b,coff+16)[0]
opt=coff+20
if struct.unpack_from('<H',b,opt)[0] != 0x10b: raise SystemExit('not PE32')
section_align=struct.unpack_from('<I',b,opt+32)[0]
file_align=struct.unpack_from('<I',b,opt+36)[0]
sect_table=opt+szopt
first_raw=min(struct.unpack_from('<I',b,sect_table+i*40+20)[0] for i in range(nsects) if struct.unpack_from('<I',b,sect_table+i*40+20)[0])
new_sh_off=sect_table+nsects*40
if new_sh_off+40 > first_raw: raise SystemExit('no header room')

def align(x,a): return (x+a-1)//a*a
last_va_end=0
last_raw_end=0
for i in range(nsects):
    o=sect_table+i*40
    vs,va,rs,rp=struct.unpack_from('<IIII',b,o+8)
    last_va_end=max(last_va_end, va+max(vs,rs))
    last_raw_end=max(last_raw_end, rp+rs)
new_va=align(last_va_end,section_align)
new_raw=align(max(last_raw_end,len(b)),file_align)

# Resource builder uses offsets relative to .rsrc start.
r=bytearray()
def alloc(n,align_to=1):
    while len(r)%align_to: r.append(0)
    off=len(r); r.extend(b'\0'*n); return off

def put_dir(named,idc):
    off=alloc(16,4)
    struct.pack_into('<IIHHHH',r,off,0,0,0,0,named,idc)
    return off

def put_entries(n): return alloc(8*n,4)
def put_wstr(s):
    enc=s.encode('utf-16le'); off=alloc(2+len(enc),2)
    struct.pack_into('<H',r,off,len(s)); r[off+2:off+2+len(enc)]=enc; return off

def put_data_entry(): return alloc(16,4)

# Root -> named type DATA
root=put_dir(1,0); root_e=put_entries(1)
type_dir=put_dir(len(resources),0); type_e=put_entries(len(resources))
# level 3 directories and entries
lang_dirs=[]; lang_entries=[]; data_entries=[]
for _ in resources:
    d=put_dir(0,1); e=put_entries(1); de=put_data_entry(); lang_dirs.append(d); lang_entries.append(e); data_entries.append(de)
# strings
s_data=put_wstr('DATA')
name_strs=[put_wstr(name) for name,_ in resources]
# raw data
raw_offsets=[]
for name,data in resources:
    off=alloc(len(data),4); r[off:off+len(data)]=data; raw_offsets.append(off)
# fill entries
struct.pack_into('<II',r,root_e,0x80000000|s_data,0x80000000|type_dir)
for i,(name,data) in enumerate(resources):
    struct.pack_into('<II',r,type_e+i*8,0x80000000|name_strs[i],0x80000000|lang_dirs[i])
    struct.pack_into('<II',r,lang_entries[i],3081,data_entries[i])
    struct.pack_into('<IIII',r,data_entries[i],new_va+raw_offsets[i],len(data),0,0)

virt_size=len(r); raw_size=align(virt_size,file_align)
r.extend(b'\0'*(raw_size-len(r)))
# pad file to raw start
if len(b)<new_raw: b.extend(b'\0'*(new_raw-len(b)))
b.extend(r)

# Write new section header
name=b'.rsrc\0\0\0'
sh=bytearray(40); sh[:8]=name
struct.pack_into('<IIIIIIHHI',sh,8,virt_size,new_va,raw_size,new_raw,0,0,0,0,0x40000040)
b[new_sh_off:new_sh_off+40]=sh
# update number sections
struct.pack_into('<H',b,coff+2,nsects+1)
# resource directory PE32 opt+96 + index 2
struct.pack_into('<II',b,opt+96+2*8,new_va,virt_size)
# SizeOfInitializedData += raw_size
old_init=struct.unpack_from('<I',b,opt+8)[0]
struct.pack_into('<I',b,opt+8,old_init+raw_size)
# SizeOfImage
struct.pack_into('<I',b,opt+56,align(new_va+virt_size,section_align))
# checksum remains zero
out.write_bytes(b)
print(f'wrote {out} size={len(b)} rsrc_va=0x{new_va:x} rsrc_raw=0x{new_raw:x} virt={virt_size} raw={raw_size}')
