#ifndef NEMOMODELS_H
#define NEMOMODELS_H

#if defined(BUILD_NEMO_QML_PLUGIN_MODELS_LIB)
    #define NEMO_QML_PLUGIN_MODELS_EXPORT Q_DECL_EXPORT
#else
    #define NEMO_QML_PLUGIN_MODELS_EXPORT Q_DECL_IMPORT
#endif

#endif // NEMOMODELS_H