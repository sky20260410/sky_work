import { ref, computed, watch } from 'vue'

const STORAGE_KEY = 'weight-records'

export interface WeightRecord {
  date: string  // YYYY-MM-DD
  weight: number
}

const records = ref<WeightRecord[]>([])
const targetWeight = ref<number | null>(null)
const initialized = ref(false)

function loadFromStorage() {
  try {
    const saved = localStorage.getItem(STORAGE_KEY)
    if (saved) {
      records.value = JSON.parse(saved)
    }
    const savedTarget = localStorage.getItem('weight-target')
    if (savedTarget) {
      targetWeight.value = parseFloat(savedTarget)
    }
  } catch (e) {
    console.error('Failed to load data:', e)
  }
  initialized.value = true
}

function saveToStorage() {
  if (!initialized.value) return
  localStorage.setItem(STORAGE_KEY, JSON.stringify(records.value))
  if (targetWeight.value !== null) {
    localStorage.setItem('weight-target', targetWeight.value.toString())
  }
}

watch(records, saveToStorage, { deep: true })
watch(targetWeight, saveToStorage)

loadFromStorage()

export function useWeight() {
  const today = computed(() => {
    const d = new Date()
    return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, '0')}-${String(d.getDate()).padStart(2, '0')}`
  })

  const todayRecord = computed(() => {
    return records.value.find(r => r.date === today.value)
  })

  const latestWeight = computed(() => {
    if (records.value.length === 0) return null
    const sorted = [...records.value].sort((a, b) => b.date.localeCompare(a.date))
    return sorted[0].weight
  })

  const diffFromTarget = computed(() => {
    if (targetWeight.value === null || latestWeight.value === null) return null
    return latestWeight.value - targetWeight.value
  })

  const progress = computed(() => {
    if (targetWeight.value === null || latestWeight.value === null) return null
    const startWeight = records.value.length > 0
      ? [...records.value].sort((a, b) => a.date.localeCompare(b.date))[0].weight
      : latestWeight.value
    if (startWeight === targetWeight.value) return 100
    const current = Math.abs(diffFromTarget.value || 0)
    const total = Math.abs(startWeight - targetWeight.value)
    const progress = ((total - current) / total) * 100
    return Math.max(0, Math.min(100, progress))
  })

  const sortedRecords = computed(() => {
    return [...records.value].sort((a, b) => b.date.localeCompare(a.date))
  })

  const chartData = computed(() => {
    const sorted = [...records.value].sort((a, b) => a.date.localeCompare(b.date))
    return {
      labels: sorted.map(r => r.date.slice(5)),
      values: sorted.map(r => r.weight)
    }
  })

  function setTarget(weight: number) {
    targetWeight.value = weight
  }

  function addOrUpdateWeight(weight: number) {
    const existing = records.value.findIndex(r => r.date === today.value)
    if (existing >= 0) {
      records.value[existing].weight = weight
    } else {
      records.value.push({ date: today.value, weight })
    }
  }

  function updateRecord(date: string, weight: number) {
    const idx = records.value.findIndex(r => r.date === date)
    if (idx >= 0) {
      records.value[idx].weight = weight
    }
  }

  function deleteRecord(date: string) {
    records.value = records.value.filter(r => r.date !== date)
  }

  function getRecord(date: string): WeightRecord | undefined {
    return records.value.find(r => r.date === date)
  }

  return {
    records,
    targetWeight,
    today,
    todayRecord,
    latestWeight,
    diffFromTarget,
    progress,
    sortedRecords,
    chartData,
    setTarget,
    addOrUpdateWeight,
    updateRecord,
    deleteRecord,
    getRecord
  }
}
