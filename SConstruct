
source  = ['Source/zif.cpp',
           'Source/ZConsole.cpp',
           'Source/ZStream.cpp']

binary  = 'zif'
app     = 'Zif'
version = '0.5.0'

# Get a build environment
env,lib = SConscript('Platform/build.scons', ['app', 'version'])
env.Append(CCFLAGS = ['-DTERMINAL_EMULATOR'])

# Project specific build config
debug = ARGUMENTS.get('debug', 0)
if int(debug) == 0:
   env.Append(CCFLAGS = ['-O3', '-DNDEBUG'])
else:
   env.Append(CCFLAGS = ['-O0', '-g'])

# Builders
exe = env.Program(binary, source)
Depends(exe, lib)

env.Tar(app+'_'+env['target']+'_'+env['machine']+'_'+version+'.tgz',
        [exe, env['platform_files'], 'LICENSE', 'zif.cfg', 'TermConfig.xml', 'Games'])

