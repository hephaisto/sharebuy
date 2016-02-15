debug=int(ARGUMENTS.get("debug",0))

if debug==0:
    SConscript("src/SConscript", variant_dir="Release"
        #, duplicate=0
        )
else:
    SConscript("src/SConscript", variant_dir="Debug"
        #, duplicate=0
        )
