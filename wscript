# @file wscript
# @brief waf build script
# @author Jeff Perry <jeffsp@gmail.com>
# @version 1.0
# @date 2013-04-30

# waf project directories
top = '.'
out = 'build'

# global definitions
SOURCES='*.cc'
CXXFLAGS=['-fopenmp','-Wall','-std=c++0x']
INCLUDES=''
LIBS=['gomp','rt','sensors']

# variant specific build flags
DEBUG_CXXFLAGS=CXXFLAGS+['-g']
RELEASE_CXXFLAGS=CXXFLAGS+['-O2','-DNDEBUG']

import glob

def configure(ctx):

    ctx.setenv('debug')
    ctx.load('compiler_cxx')
    ctx.env.CXXFLAGS=DEBUG_CXXFLAGS
    ctx.env.SOURCES=glob.glob(SOURCES)
    ctx.check_cfg(package='gtk+-3.0',args=['--cflags','--libs'])

    ctx.setenv('release')
    ctx.load('compiler_cxx')
    ctx.env.CXXFLAGS=RELEASE_CXXFLAGS
    ctx.env.SOURCES=glob.glob(SOURCES)
    ctx.check_cfg(package='gtk+-3.0',args=['--cflags','--libs'])

def options(opt):

    opt.load('compiler_cxx')

def init(ctx):

    # setup contexts build_debug, build_release, clean_debug, ...
    from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext
    for x in (BuildContext, CleanContext, InstallContext, UninstallContext):
        for y in ['debug','release']:
            class tmp(x):
                variant=y
                cmd=x.__name__.replace('Context','').lower()+'_'+y

def build(ctx):

    # if no variant was specified then build them all
    if not ctx.variant:
        import waflib.Options
        for x in ['debug', 'release']:
            waflib.Options.commands.insert(0, ctx.cmd+'_'+x)
    else:
        # the executable name is the filename without the extension
        ctx.program(source='proctempalert.cc',target='proctempalert',includes=INCLUDES,lib=LIBS)
        ctx.program(source='proctempview.cc',target='proctempview',includes=INCLUDES,lib=LIBS+['ncurses'])
