
source  = ['Source/zif.cpp',
           'Source/ConsoleImpl.cpp',
           'Source/Z/ZStream.cpp']

binary  = 'zif'
app     = 'Zif'
version = '0.5.4'

# Get a build environment
env,libs = SConscript('Platform/build.scons', ['app', 'version'])
env.Append(CCFLAGS = ['-DTERMINAL_EMULATOR'])

# Project specific build config
debug = ARGUMENTS.get('debug', 0)
if int(debug) == 0:
   env.Append(CCFLAGS = ['-O3', '-DNDEBUG'])
else:
   env.Append(CCFLAGS = ['-O0', '-g'])

env.Append(CPPPATH = ['Source'])

# Builders
exe = env.Program(binary, source)
Depends(exe, libs)

env.Tar(app+'_'+env['target']+'_'+env['machine']+'_'+version+'.tgz',
        [exe, env['platform_files'], 'LICENSE', 'zif.cfg', 'TermConfig.xml', 'Games'])

