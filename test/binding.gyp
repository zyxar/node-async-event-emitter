{
  'targets': [{
    'target_name': 'addon',
    'sources': [ 'addon.cc',
      "<!(node -e \"require('..')\")/AsyncCallback.cc",
      "<!(node -e \"require('..')\")/CrossCallback.cc"
    ],
    'include_dirs' : ["<!(node -e \"require('..')\")"],
    'conditions': [
      [ 'OS=="mac"', {
        'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',       # -fno-exceptions
          'GCC_ENABLE_CPP_RTTI': 'YES',             # -fno-rtti
          'MACOSX_DEPLOYMENT_TARGET' : '10.7',      # from MAC OS 10.7
          'OTHER_CFLAGS': [
            '-g -O3 -stdlib=libc++ -std=c++11',
            '-Wno-unused-private-field',
          ]
        },
      }, { # OS!="mac"
        'cflags!':    ['-fno-exceptions'],
        'cflags':     ['-D__STDC_CONSTANT_MACROS'],
        'cflags_cc':  ['-Wall', '-O3', '-g' , '-std=c++11', '-fexceptions'],
        'cflags_cc!': ['-fno-exceptions'],
        'cflags_cc!': ['-fno-rtti'],
      }],
    ]
  }]
}
