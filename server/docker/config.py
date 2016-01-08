import os, sys, json

def get_value(j, key):
    return j[key] if j.has_key(key) else ''

def get(key):
    try:
        filepath = os.path.join(os.path.dirname(os.path.abspath(sys.argv[0])), 'config.json')
        j = json.loads(file(filepath).read())
    except:
        j = {}

    s = get_value(j, key)
    if s:
        return s

    # PROJECT_NAME is a required value. Provide a default value if necessary.
    project_name = get_value(j, 'PROJECT_NAME')
    if not project_name:
        project_name = 'server'

    if key == 'PROJECT_NAME':
        return project_name

    if key == 'REGISTRY_PREFIX':
        registry_url = get_value(j, 'REGISTRY_URL')
        if not registry_url:
            return project_name
        return '/'.join([registry_url, project_name])

    return ''
